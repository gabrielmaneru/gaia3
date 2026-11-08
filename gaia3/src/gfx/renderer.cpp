#include "gaia3_pch.h"
#include "renderer.h"

#include <glad/glad.h>

#include <algorithm>

namespace
{
void LogOpenGLInfo()
{
	auto getString = []( u32 name ) -> const char* {
		return reinterpret_cast<const char*>( glGetString( name ) );
	};

	auto getInteger = []( u32 name ) -> int {
		int value = 0;
		glGetIntegerv( name, &value );
		return value;
	};

	GAIA_INFO( "OpenGL vendor: {}", getString( GL_VENDOR ) );
	GAIA_INFO( "OpenGL renderer: {}", getString( GL_RENDERER ) );
	GAIA_INFO( "OpenGL version: {}", getString( GL_VERSION ) );
	GAIA_INFO( "GLSL version: {}", getString( GL_SHADING_LANGUAGE_VERSION ) );
	GAIA_INFO( "OpenGL extensions: {}", getInteger( GL_NUM_EXTENSIONS ) );
	GAIA_INFO( "OpenGL max texture size: {}", getInteger( GL_MAX_TEXTURE_SIZE ) );
	GAIA_INFO( "OpenGL max texture units: {}", getInteger( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ) );
	GAIA_INFO( "OpenGL max vertex attributes: {}", getInteger( GL_MAX_VERTEX_ATTRIBS ) );
	GAIA_INFO( "OpenGL max uniform block size: {}", getInteger( GL_MAX_UNIFORM_BLOCK_SIZE ) );
}
} // namespace

bool Renderer::Init( ProcAddressFn procAddressLoader, rect canvas )
{
	GAIA_ASSERT( procAddressLoader != nullptr );
	GAIA_ASSERT( canvas.x > 0 && canvas.y > 0 );

	if( !gladLoadGLLoader( reinterpret_cast<GLADloadproc>( procAddressLoader ) ) )
	{
		GAIA_ERROR( "Renderer init failed: GLAD loader initialization failed" );
		return false;
	}

	LogOpenGLInfo();

	if( !m_RenderTarget.Create( canvas ) )
	{
		GAIA_ERROR( "Renderer init failed: Failed to create the render target" );
		return false;
	}

	glEnable( GL_DEPTH_TEST );

	return true;
}

void Renderer::Update( rect renderSize )
{
	if( !m_RenderTarget.Resize( renderSize ) )
	{
		GAIA_ERROR( "Renderer: failed to resize the render target to {}x{}", renderSize.x, renderSize.y );
		return;
	}

	m_RenderTarget.Bind();
	glViewport( 0, 0, renderSize.x, renderSize.y );

	// Clear buffer
	glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Note: Without a camera, view and projection stay identity
	DrawContext context;

	const RenderCamera* camera = GetCamera();
	if( camera != nullptr )
	{
		context.view       = camera->GetViewMatrix();
		context.projection = camera->GetProjectionMatrix( renderSize );
	}

	for( const Renderable* renderable : m_Renderables )
	{
		context.model = renderable->GetModelMatrix();
		renderable->Draw( context );
	}

	Framebuffer::Unbind();
}

void Renderer::PresentToWindow( rect canvas ) const
{
	m_RenderTarget.BlitToWindow( canvas );
}

void Renderer::Shutdown()
{
	m_Renderables.clear();
	m_Cameras.clear();

	m_RenderTarget.Destroy();
}
