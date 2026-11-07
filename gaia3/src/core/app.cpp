#include "gaia3_pch.h"
#include "app.h"

Window         App::m_Window;
Input          App::m_Input;
Renderer       App::m_Renderer;
Session        App::m_Session;
unique<Editor> App::m_Editor;
u64            App::m_MaxFrames = 0;

bool App::Init( const AppConfig& config )
{
	m_MaxFrames = config.maxFrames;

	if( !m_Window.Init( config.title, config.canvas ) )
		return false;

	if( !m_Input.Init( m_Window.GetWindowHandle() ) )
		return false;

	if( !m_Renderer.Init( m_Window.GetProcAddressLoader(), m_Window.GetCanvas() ) )
		return false;

	if( !m_Session.Init( config.startLevel, config.commonPath, config.appPath ) )
		return false;

	if( config.editor.enabled )
	{
		m_Editor = new_unique<Editor>( config.editor );
		if( !m_Editor->Init( config.commonPath ) )
		{
			GAIA_ERROR( "Editor failed to initialize, continuing without it" );
			m_Editor.reset();
		}
	}

	GAIA_INFO( "{} initialized", config.title );
	return true;
}

void App::Run()
{
	u64 frameCount = 0;
	do
	{
		if( !m_Window.IsMinimized() )
		{
			if( m_Editor != nullptr )
			{
				m_Input.SetCaptured( m_Editor->WantsMouse(), m_Editor->WantsKeyboard() );
			}

			m_Session.Update();

			const rect canvas         = m_Window.GetCanvas();
			const bool bEditorDisplay = m_Editor != nullptr && m_Editor->DisplaysRenderTarget();

			m_Renderer.Update( bEditorDisplay ? m_Editor->GetViewportSize() : canvas );
			if( !bEditorDisplay )
			{
				m_Renderer.PresentToWindow( canvas );
			}

			if( m_Editor != nullptr )
			{
				m_Editor->Draw();
			}

			m_Input.Update();
		}
		m_Window.Update();

		++frameCount;
	} while( !m_Window.ShouldClose() && ( m_MaxFrames == 0 || frameCount < m_MaxFrames ) );
}

void App::Shutdown()
{
	if( m_Editor != nullptr )
	{
		m_Editor->Shutdown();
		m_Editor.reset();
	}

	m_Session.Shutdown();
	m_Renderer.Shutdown();
	m_Window.Shutdown();
}
