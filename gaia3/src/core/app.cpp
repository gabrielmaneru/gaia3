#include "gaia3_pch.h"
#include "app.h"

Window App::m_Window;
Input App::m_Input;
Renderer App::m_Renderer;

bool App::Init()
{
	if (!m_Window.Init("Gaia3", { 800, 600 }))
		return false;

	if (!m_Input.Init(m_Window.GetWindowHandle()))
		return false;

	if (!m_Renderer.Init(m_Window.GetProcAddressLoader(), m_Window.GetCanvas()))
		return false;

	return true;
}

void App::Run()
{
	do
	{
		if (!m_Window.IsMinimized())
		{
			m_Renderer.Update(m_Window.GetCanvas());
			m_Input.Update();
		}
		m_Window.Update();
	} while (!m_Window.ShouldClose());
}

void App::Shutdown()
{
	m_Renderer.Shutdown();
	m_Window.Shutdown();
}
