// Note: Draws a material's colour, multiplied by its texture when it has one, lit by the scene's lights

#type vertex
#version 330 core

layout( location = 0 ) in vec3 aPosition;
layout( location = 1 ) in vec3 aNormal;
layout( location = 2 ) in vec2 aUV;
layout( location = 3 ) in vec4 aTangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3  vWorldPosition;
out vec3  vNormal;
out vec3  vTangent;
out float vBitangentSign;
out vec2  vUV;

void main()
{
	vec4 worldPosition = u_Model * vec4( aPosition, 1.0 );

	vWorldPosition = worldPosition.xyz;
	vNormal        = mat3( u_Model ) * aNormal;
	vTangent       = mat3( u_Model ) * aTangent.xyz;
	vBitangentSign = aTangent.w;
	vUV            = aUV;
	gl_Position    = u_Projection * u_View * worldPosition;
}

#type fragment
#version 330 core

in vec3  vWorldPosition;
in vec3  vNormal;
in vec3  vTangent;
in float vBitangentSign;
in vec2  vUV;

// Note: Must match kMaxLights and LightType in gfx/rendererTypes.h
const int kMaxLights        = 8;
const int kDirectionalLight = 0;
const int kPointLight       = 1;

// Note: Must match DrawMode in gfx/rendererTypes.h
const int kDrawLit     = 0;
const int kDrawNormals = 1;

uniform int u_DrawMode;

// Note: Light that reaches every surface, so places no light reaches aren't black, until there is a sky or bounce light
const float kAmbient = 0.15;

uniform int   u_LightCount;
uniform int   u_LightType[kMaxLights];
uniform vec3  u_LightColor[kMaxLights];
uniform vec3  u_LightPosition[kMaxLights];
uniform vec3  u_LightDirection[kMaxLights];
uniform float u_LightRange[kMaxLights];

uniform vec3      u_Color;
uniform bool      u_UseTexture;
uniform sampler2D u_Texture;

uniform bool      u_AlphaTest;
uniform bool      u_UseOpacityTexture;
uniform bool      u_OpacityFromAlpha;
uniform sampler2D u_OpacityTexture;

uniform bool      u_UseNormalTexture;
uniform sampler2D u_NormalTexture;

out vec4 FragColor;

// Note: The surface's normal, bent by the normal map when there is one. A normal map stores a direction relative to
// the surface, x along the tangent, y along the bitangent and z along the normal, each squeezed from -1..1 into 0..1
vec3 GetFrontNormal()
{
	vec3 normal = normalize( vNormal );
	if( !u_UseNormalTexture )
		return normal;

	// Note: Interpolation can tilt the tangent off the surface, so it is straightened against the normal first
	vec3 tangent   = normalize( vTangent - normal * dot( normal, vTangent ) );
	vec3 bitangent = cross( normal, tangent ) * vBitangentSign;

	vec3 mapNormal = texture( u_NormalTexture, vUV ).rgb * 2.0 - 1.0;
	return normalize( mat3( tangent, bitangent, normal ) * mapNormal );
}

// Note: The back of a face, only drawn for two-sided materials, faces the other way and is lit as its own surface
vec3 GetSurfaceNormal()
{
	vec3 normal = GetFrontNormal();
	return gl_FrontFacing ? normal : -normal;
}

void main()
{
	vec4 texel  = u_UseTexture ? texture( u_Texture, vUV ) : vec4( 1.0 );
	vec3 albedo = u_Color * texel.rgb;

	// Note: Cut-out materials, like leaves, skip the pixels their mask marks as see-through. A mask with an alpha
	// channel is read from its alpha, a grey mask from its red channel, and without a mask the colour texture's alpha
	if( u_AlphaTest )
	{
		float opacity = texel.a;
		if( u_UseOpacityTexture )
		{
			vec4 mask = texture( u_OpacityTexture, vUV );
			opacity   = u_OpacityFromAlpha ? mask.a : mask.r;
		}

		if( opacity < 0.5 )
		{
			discard;
		}
	}

	vec3 normal = GetSurfaceNormal();

	// Note: Each axis from -1..1 into 0..1, so +X is red, +Y green and +Z blue
	if( u_DrawMode == kDrawNormals )
	{
		FragColor = vec4( normal * 0.5 + 0.5, 1.0 );
		return;
	}

	vec3 lighting = vec3( kAmbient );

	for( int i = 0; i < u_LightCount; ++i )
	{
		vec3  toLight     = -u_LightDirection[i];
		float attenuation = 1.0;

		// Note: Point lights fade with distance and reach exactly zero at their range, so they end smoothly
		if( u_LightType[i] == kPointLight )
		{
			vec3  offset   = u_LightPosition[i] - vWorldPosition;
			float distance = length( offset );
			float fade     = clamp( 1.0 - pow( distance / max( u_LightRange[i], 0.001 ), 4.0 ), 0.0, 1.0 );

			toLight     = offset / max( distance, 0.0001 );
			attenuation = fade * fade / ( distance * distance + 1.0 );
		}

		lighting += u_LightColor[i] * max( dot( normal, toLight ), 0.0 ) * attenuation;
	}

	FragColor = vec4( albedo * lighting, 1.0 );
}
