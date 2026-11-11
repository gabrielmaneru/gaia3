#pragma once

#include <gfx/texture2D.h>
#include <math/matrix.h>

// Note: How the sun's shadow map sees the world, see FitShadowToBox
struct ShadowView
{
	// Note: From world space to the shadow map's clip space
	Matrix44 viewProjection;

	// Note: The width of one shadow map texel in world units
	float texelWorldSize = 0.0f;
};

// Note: A box seen along lightDirection that starts at lightPosition, centred on it across the light, size wide and
// tall, and runs depth along the light. Casters behind the light still cast, the shadow pass clamps their depth to the
// box instead of cutting them away
ShadowView FitShadowToBox( const Vec3& lightPosition, const Vec3& lightDirection, float size, float depth, u32 mapSize );

// Note: A square depth-only render target for the sun's shadow pass. Call Destroy while the OpenGL context is alive,
// the destructor does not release anything
class ShadowMap
{
public:
	// Note: Creates it on first use and again when the size changes
	bool Resize( u32 size );
	void Destroy();

	void Bind() const;

	u32 GetDepthTexture() const { return m_Depth.GetId(); }
	u32 GetSize() const { return static_cast<u32>( m_Depth.GetSize().x ); }

private:
	u32       m_Framebuffer = 0;
	Texture2D m_Depth;
};
