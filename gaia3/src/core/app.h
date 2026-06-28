#pragma once

#include <core/window.h>

class App
{
public:
	static void Init();
	static void Run();
	static void Shutdown();

private:
	static Window m_Window;
};