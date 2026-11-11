// Note: Draws a material's colour, multiplied by its texture when it has one, lit by the scene's lights with
// metallic-roughness PBR and shadowed by the sun's shadow map. Lighting is done in linear colour, then tone mapped

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
const int kDrawLit       = 0;
const int kDrawNormals   = 1;
const int kDrawShadows   = 2;
const int kDrawRoughness = 3;
const int kDrawMetallic  = 4;

uniform int u_DrawMode;

// Note: The sun's shadow map. u_ShadowLight is the index of the light it belongs to, -1 without one
uniform int             u_ShadowLight;
uniform sampler2DShadow u_ShadowMap;
uniform mat4            u_ShadowMatrix;
uniform float           u_ShadowDepthBias;
uniform float           u_ShadowNormalOffset;

// Note: Light that reaches every surface, so places no light reaches aren't black, until there is a sky or bounce light.
// It is in linear light, so it shows about as bright as 0.15 did before lighting moved to linear colour
const float kAmbient = 0.02;

uniform vec3 u_CameraPosition;

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

// Note: Metallic-roughness PBR, each value multiplied by its texture's red channel when there is one, see pbr.glsli
uniform float     u_Roughness;
uniform bool      u_UseRoughnessTexture;
uniform sampler2D u_RoughnessTexture;
uniform float     u_Metallic;
uniform bool      u_UseMetallicTexture;
uniform sampler2D u_MetallicTexture;

out vec4 FragColor;

#include "pbr.glsli"

// Note: Colour textures are stored for the eye (sRGB), lighting adds light up in linear values, and the screen expects
// sRGB again. A 2.2 power is close enough to the exact sRGB curve
vec3 SrgbToLinear( vec3 color )
{
	return pow( color, vec3( 2.2 ) );
}

vec3 LinearToSrgb( vec3 color )
{
	return pow( color, vec3( 1.0 / 2.2 ) );
}

// Note: Rolls bright values off smoothly instead of clipping them at 1, Krzysztof Narkowicz's fit of the ACES curve
vec3 ToneMap( vec3 color )
{
	return clamp( ( color * ( 2.51 * color + 0.03 ) ) / ( color * ( 2.43 * color + 0.59 ) + 0.14 ), 0.0, 1.0 );
}

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

// Note: How much of the shadowed light reaches this pixel, 0 in full shadow and 1 fully lit. The point is first moved
// out along its face's normal, so the surface doesn't shadow itself, and 9 lookups around it soften the edge. Each
// lookup compares depths in hardware and blends the 4 nearest texels
float GetShadow()
{
	vec3 faceNormal = normalize( vNormal ) * ( gl_FrontFacing ? 1.0 : -1.0 );
	vec4 position   = u_ShadowMatrix * vec4( vWorldPosition + faceNormal * u_ShadowNormalOffset, 1.0 );
	vec3 coords     = position.xyz / position.w * 0.5 + 0.5;

	// Note: Outside the shadow map nothing is known about the light, so the pixel is lit
	if( coords.x < 0.0 || coords.x > 1.0 || coords.y < 0.0 || coords.y > 1.0 || coords.z > 1.0 )
		return 1.0;

	vec2  texelSize = 1.0 / vec2( textureSize( u_ShadowMap, 0 ) );
	float lit       = 0.0;
	for( int x = -1; x <= 1; ++x )
	{
		for( int y = -1; y <= 1; ++y )
		{
			lit += texture( u_ShadowMap, vec3( coords.xy + vec2( x, y ) * texelSize, coords.z - u_ShadowDepthBias ) );
		}
	}
	return lit / 9.0;
}

void main()
{
	vec4 texel  = u_UseTexture ? texture( u_Texture, vUV ) : vec4( 1.0 );
	vec3 albedo = u_Color * SrgbToLinear( texel.rgb );

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

	float shadow = u_ShadowLight >= 0 ? GetShadow() : 1.0;
	if( u_DrawMode == kDrawShadows )
	{
		FragColor = vec4( vec3( shadow ), 1.0 );
		return;
	}

	// Note: Data textures hold values, not colours, so they are read as they are
	float roughness = u_Roughness * ( u_UseRoughnessTexture ? texture( u_RoughnessTexture, vUV ).r : 1.0 );
	float metallic  = u_Metallic * ( u_UseMetallicTexture ? texture( u_MetallicTexture, vUV ).r : 1.0 );
	if( u_DrawMode == kDrawRoughness )
	{
		FragColor = vec4( vec3( roughness ), 1.0 );
		return;
	}
	if( u_DrawMode == kDrawMetallic )
	{
		FragColor = vec4( vec3( metallic ), 1.0 );
		return;
	}

	vec3 toEye = normalize( u_CameraPosition - vWorldPosition );
	vec3 color = albedo * kAmbient;

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

		// Note: The ambient light above is left unshadowed
		if( i == u_ShadowLight )
		{
			attenuation *= shadow;
		}

		color += GetPbrLighting( normal, toEye, toLight, albedo, roughness, metallic ) * u_LightColor[i] * attenuation;
	}

	FragColor = vec4( LinearToSrgb( ToneMap( color ) ), 1.0 );
}
