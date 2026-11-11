#include "gaia3_pch.h"
#include "shadowMap.h"

#include <math/matrixTransforms.h>
#include <math/quaternionTransforms.h>
#include <math/transform.h>

#include <glad/glad.h>

#include <algorithm>

ShadowView FitShadowToBox( const Vec3& lightPosition, const Vec3& lightDirection, float size, float depth, u32 mapSize )
{
	// Note: A box with no width or depth can't be projected, so it keeps a small minimum
	const float halfSize = std::max( size, 0.01f ) * 0.5f;

	const Quat     lightRotation = LookAtQuat( lightDirection, Vec3::V_Up );
	const Matrix44 lightView     = Transform( lightPosition, lightRotation, Vec3::V_One ).Inverse().ToMatrix44();

	// Note: The view looks down -Z from the light, so the box runs from the light to depth along -Z
	const Matrix44 projection = Orthographic( -halfSize, halfSize, -halfSize, halfSize, 0.0f, std::max( depth, 0.01f ) );

	ShadowView shadowView;
	shadowView.viewProjection = projection * lightView;
	shadowView.texelWorldSize = 2.0f * halfSize / static_cast<float>( mapSize );
	return shadowView;
}

bool ShadowMap::Resize( u32 size )
{
	if( m_Framebuffer != 0 && GetSize() == size )
		return true;

	Destroy();

	if( !m_Depth.Create( rect{ static_cast<int>( size ), static_cast<int>( size ) }, TextureProperties::V_ShadowMap ) )
		return false;

	glGenFramebuffers( 1, &m_Framebuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, m_Framebuffer );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth.GetId(), 0 );

	// Note: Depth only, there is no colour to write or read
	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );

	const u32 status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		GAIA_ERROR( "ShadowMap: incomplete at {}x{}, status {:#x}", size, size, status );
		Destroy();
		return false;
	}

	return true;
}

void ShadowMap::Destroy()
{
	if( m_Framebuffer != 0 )
	{
		glDeleteFramebuffers( 1, &m_Framebuffer );
		m_Framebuffer = 0;
	}

	m_Depth.Destroy();
}

void ShadowMap::Bind() const
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_Framebuffer );
}
