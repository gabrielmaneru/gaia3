#pragma once

#include <core/window.h>
#include <gfx/renderer.h>

class App
{
public:
	static void Init();
	static void Run();
	static void Shutdown();

private:
	static Window m_Window;
	static Renderer m_Renderer;
};