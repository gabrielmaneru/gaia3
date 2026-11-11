#include "gaia3_pch.h"
#include "renderer.h"

#include <core/app.h>
#include <gfx/shader.h>

#include <glad/glad.h>

#include <algorithm>

namespace
{
constexpr const char* kShadowShaderName = "shadow";

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

	// Note: Faces are counter-clockwise seen from their front, like the engine's models, and their backs aren't drawn.
	// Two-sided materials turn culling off while they draw, see MaterialEntry::Bind
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );

	return true;
}

void Renderer::Update( rect renderSize )
{
	if( !m_RenderTarget.Resize( renderSize ) )
	{
		GAIA_ERROR( "Renderer: failed to resize the render target to {}x{}", renderSize.x, renderSize.y );
		return;
	}

	// Note: Without a camera, view and projection stay identity
	DrawContext context;

	const RenderCamera* camera = GetCamera();
	if( camera != nullptr )
	{
		context.view           = camera->GetViewMatrix();
		context.projection     = camera->GetProjectionMatrix( renderSize );
		context.cameraPosition = Vec3( context.view.Inverse().col[3] );
	}

	context.lights     = GatherLights();
	context.drawMode   = m_DrawMode;
	context.bAlphaTest = m_bAlphaTest;

	DrawShadowMap( context );

	m_RenderTarget.Bind();
	glViewport( 0, 0, renderSize.x, renderSize.y );

	// Clear buffer
	glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	for( const Renderable* renderable : m_Renderables )
	{
		context.model = renderable->GetModelMatrix();
		renderable->Draw( context );
	}

	Framebuffer::Unbind();
}

void Renderer::DrawShadowMap( DrawContext& context )
{
	context.shadowLight = -1;
	m_ShadowCaster      = nullptr;

	const int shadowLight = FindShadowLight( context.lights );
	if( !m_ShadowSettings.bEnabled || shadowLight < 0 )
		return;

	if( m_ShadowShader == nullptr )
	{
		m_ShadowShader = App::GetSession().GetResourceManager().FindResource<Shader>( kShadowShaderName, true );
		if( m_ShadowShader == nullptr )
		{
			// Note: Reported once, it is looked up again every frame in case it becomes available
			if( !m_bReportedShadowShader )
			{
				GAIA_ERROR( "Renderer: the {} shader is not available, drawing without shadows", kShadowShaderName );
				m_bReportedShadowShader = true;
			}
			return;
		}
	}

	if( !m_ShadowMap.Resize( m_ShadowSettings.mapSize ) )
		return;

	const LightParameters& light      = context.lights[shadowLight];
	const ShadowView       shadowView = FitShadowToBox( light.position, light.direction, light.shadowSize, light.shadowDepth, m_ShadowSettings.mapSize );

	// Note: GatherLights keeps the registration order, so the index is the same in m_Lights
	m_ShadowCaster = m_Lights[shadowLight];

	context.shadowLight        = shadowLight;
	context.shadowMatrix       = shadowView.viewProjection;
	context.shadowMap          = m_ShadowMap.GetDepthTexture();
	context.shadowDepthBias    = m_ShadowSettings.depthBias;
	context.shadowNormalOffset = m_ShadowSettings.normalBias * shadowView.texelWorldSize;

	m_ShadowMap.Bind();
	glViewport( 0, 0, static_cast<int>( m_ShadowSettings.mapSize ), static_cast<int>( m_ShadowSettings.mapSize ) );
	glClear( GL_DEPTH_BUFFER_BIT );

	// Note: Casters between the light and the shadow map's box are flattened onto its near side instead of cut away
	glEnable( GL_DEPTH_CLAMP );

	for( const Renderable* renderable : m_Renderables )
	{
		context.model = renderable->GetModelMatrix();
		renderable->DrawShadow( context, *m_ShadowShader );
	}

	glDisable( GL_DEPTH_CLAMP );
	Framebuffer::Unbind();
}

int Renderer::FindShadowLight( const std::vector<LightParameters>& lights )
{
	for( u64 i = 0; i < lights.size(); ++i )
	{
		if( lights[i].type == LightType::Directional && lights[i].bCastShadows )
			return static_cast<int>( i );
	}

	return -1;
}

std::vector<LightParameters> Renderer::GatherLights()
{
	// Note: Warns once each time the limit is passed, not every frame
	const bool bOverLimit = m_Lights.size() > kMaxLights;
	if( bOverLimit && !m_bWarnedLightLimit )
	{
		GAIA_WARN( "Renderer: {} lights, only the first {} are used", m_Lights.size(), kMaxLights );
	}
	m_bWarnedLightLimit = bOverLimit;

	std::vector<LightParameters> lights;
	for( const RenderLight* light : m_Lights )
	{
		if( lights.size() == kMaxLights )
			break;

		lights.push_back( light->GetLightParameters() );
	}

	return lights;
}

void Renderer::PresentToWindow( rect canvas ) const
{
	m_RenderTarget.BlitToWindow( canvas );
}

void Renderer::Shutdown()
{
	m_Renderables.clear();
	m_Cameras.clear();
	m_Lights.clear();

	m_ShadowShader.reset();
	m_ShadowCaster = nullptr;
	m_ShadowMap.Destroy();
	m_RenderTarget.Destroy();
}
