#pragma once

#include <gfx/renderer.h>
#include <platform/input.h>
#include <platform/window.h>

class App
{
public:
	static bool Init();
	static void Run();
	static void Shutdown();

private:
	static Window   m_Window;
	static Input    m_Input;
	static Renderer m_Renderer;
};