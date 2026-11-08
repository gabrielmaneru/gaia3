// Note: Draws a material's colour, multiplied by its texture when it has one

#type vertex
#version 330 core

layout( location = 0 ) in vec3 aPosition;
layout( location = 1 ) in vec3 aNormal;
layout( location = 2 ) in vec2 aUV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 vNormal;
out vec2 vUV;

void main()
{
	vNormal     = mat3( u_Model ) * aNormal;
	vUV         = aUV;
	gl_Position = u_Projection * u_View * u_Model * vec4( aPosition, 1.0 );
}

#type fragment
#version 330 core

in vec3 vNormal;
in vec2 vUV;

uniform vec3      u_Color;
uniform bool      u_UseTexture;
uniform sampler2D u_Texture;

out vec4 FragColor;

void main()
{
	vec3 albedo = u_Color;
	if( u_UseTexture )
	{
		albedo *= texture( u_Texture, vUV ).rgb;
	}

	// Note: A faraway light like the sun, fixed in the shader until the engine has lights, plus some ambient
	const vec3 kSunDirection = normalize( vec3( 0.4, 0.8, 0.6 ) );
	float      shading       = 0.55 + 0.45 * max( dot( normalize( vNormal ), kSunDirection ), 0.0 );

	FragColor = vec4( albedo * shading, 1.0 );
}
