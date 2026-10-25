#include "gaia3_pch.h"
#include "app.h"

Window App::m_Window;
Renderer App::m_Renderer;

void App::Init()
{
	m_Window.Init("Gaia3", { 800, 600 });
	m_Renderer.Init(m_Window.GetProcAddressLoader());
}

void App::Run()
{
	do
	{
		m_Renderer.Update();
		m_Window.Update();
	} while (!m_Window.ShouldClose());
}

void App::Shutdown()
{
	m_Renderer.Shutdown();
	m_Window.Shutdown();
}
