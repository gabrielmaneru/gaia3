#include "gaia3_pch.h"
#include "framebuffer.h"

#include <glad/glad.h>

bool Framebuffer::Create( rect size )
{
	GAIA_ASSERT( size.x > 0 && size.y > 0 );

	if( !m_Color.Create( size, TextureProperties::V_RenderTargetRGBA ) || !m_Depth.Create( size, TextureProperties::V_DefaultDepth ) )
	{
		Destroy();
		return false;
	}

	glGenFramebuffers( 1, &m_Framebuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, m_Framebuffer );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Color.GetId(), 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth.GetId(), 0 );

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

	m_Color.Destroy();
	m_Depth.Destroy();
}

bool Framebuffer::Resize( rect size )
{
	const rect currentSize = m_Color.GetSize();
	if( size.x == currentSize.x && size.y == currentSize.y )
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
	const rect size = m_Color.GetSize();

	glBindFramebuffer( GL_READ_FRAMEBUFFER, m_Framebuffer );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	glBlitFramebuffer( 0, 0, size.x, size.y, 0, 0, canvas.x, canvas.y, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	Unbind();
}
