#include "gaia3_pch.h"
#include "app.h"

Window   App::m_Window;
Input    App::m_Input;
Renderer App::m_Renderer;
Session  App::m_Session;

bool App::Init( const AppConfig& config )
{
	if( !m_Window.Init( config.title, config.canvas ) )
		return false;

	if( !m_Input.Init( m_Window.GetWindowHandle() ) )
		return false;

	if( !m_Renderer.Init( m_Window.GetProcAddressLoader(), m_Window.GetCanvas() ) )
		return false;

	if( !m_Session.Init( config.startLevel, config.commonPath, config.appPath ) )
		return false;

	GAIA_INFO( "{} initialized", config.title );
	return true;
}

void App::Run()
{
	do
	{
		if( !m_Window.IsMinimized() )
		{
			m_Session.Update();
			m_Renderer.Update( m_Window.GetCanvas() );
			m_Input.Update();
		}
		m_Window.Update();
	} while( !m_Window.ShouldClose() );
}

void App::Shutdown()
{
	m_Session.Shutdown();
	m_Renderer.Shutdown();
	m_Window.Shutdown();
}
