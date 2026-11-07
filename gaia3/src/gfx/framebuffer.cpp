#include "gaia3_pch.h"
#include "framebuffer.h"

#include <glad/glad.h>

bool Framebuffer::Create( rect size )
{
	GAIA_ASSERT( size.x > 0 && size.y > 0 );

	m_Size = size;

	glGenTextures( 1, &m_ColorTexture );
	glBindTexture( GL_TEXTURE_2D, m_ColorTexture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glGenRenderbuffers( 1, &m_DepthBuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, m_DepthBuffer );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.x, size.y );
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	glGenFramebuffers( 1, &m_Framebuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, m_Framebuffer );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0 );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer );

	const u32 status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		GAIA_ERROR( "Framebuffer: incomplete at {}x{}, status {:#x}", size.x, size.y, status );
		Destroy();
		return false;
	}

	return true;
}

void Framebuffer::Destroy()
{
	if( m_Framebuffer != 0 )
	{
		glDeleteFramebuffers( 1, &m_Framebuffer );
		m_Framebuffer = 0;
	}

	if( m_DepthBuffer != 0 )
	{
		glDeleteRenderbuffers( 1, &m_DepthBuffer );
		m_DepthBuffer = 0;
	}

	if( m_ColorTexture != 0 )
	{
		glDeleteTextures( 1, &m_ColorTexture );
		m_ColorTexture = 0;
	}

	m_Size = {};
}

bool Framebuffer::Resize( rect size )
{
	if( size.x == m_Size.x && size.y == m_Size.y )
		return true;

	Destroy();
	return Create( size );
}

void Framebuffer::Bind() const
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_Framebuffer );
}

void Framebuffer::Unbind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Framebuffer::BlitToWindow( rect canvas ) const
{
	glBindFramebuffer( GL_READ_FRAMEBUFFER, m_Framebuffer );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	glBlitFramebuffer( 0, 0, m_Size.x, m_Size.y, 0, 0, canvas.x, canvas.y, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	Unbind();
}
