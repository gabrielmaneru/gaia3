#include "app.h"

Window App::m_Window;

void App::Init()
{
	m_Window.Init();
}

void App::Run()
{
	do
	{
		m_Window.Update();
	} while (!m_Window.ShouldClose());
}

void App::Shutdown()
{
	m_Window.Shutdown();
}
