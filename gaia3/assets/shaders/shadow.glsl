// Note: Draws depth only into the sun's shadow map. Cut-out materials still drop their see-through pixels, so leaves
// cast leaf shapes rather than whole cards

#type vertex
#version 330 core

// Note: Must match the attribute locations in gfx/model.cpp
layout( location = 0 ) in vec3 aPosition;
layout( location = 2 ) in vec2 aUV;

uniform mat4 u_Model;
uniform mat4 u_ShadowMatrix;

out vec2 vUV;

void main()
{
	vUV         = aUV;
	gl_Position = u_ShadowMatrix * u_Model * vec4( aPosition, 1.0 );
}

#type fragment
#version 330 core

in vec2 vUV;

uniform bool      u_UseTexture;
uniform sampler2D u_Texture;

uniform bool      u_AlphaTest;
uniform bool      u_UseOpacityTexture;
uniform bool      u_OpacityFromAlpha;
uniform sampler2D u_OpacityTexture;

// Note: The same cut-out rule as shaders/default.glsl
void main()
{
	if( u_AlphaTest )
	{
		float opacity = u_UseTexture ? texture( u_Texture, vUV ).a : 1.0;
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
}
