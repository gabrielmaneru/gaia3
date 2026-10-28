#pragma once

#include <core/window.h>
#include <core/input.h>
#include <gfx/renderer.h>

class App
{
public:
	static bool Init();
	static void Run();
	static void Shutdown();

private:
	static Window m_Window;
	static Input m_Input;
	static Renderer m_Renderer;
};