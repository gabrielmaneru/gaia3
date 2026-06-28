#include "window.h"
#include <iostream>

#include <GLFW/glfw3.h>

void Window::Init()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		m_Window = nullptr;
		return;
	}

	// Request an OpenGL 3.3 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	const int width = 800;
	const int height = 600;
	const char* title = "Gaia3";

	m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!m_Window)
	{
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(1); // enable vsync
}

void Window::Update()
{
	if (!m_Window)
		return;

	// Present and poll events. If you're not using OpenGL, remove SwapBuffers.
	glfwSwapBuffers(m_Window);
	glfwPollEvents();
}

void Window::Shutdown()
{
	if (m_Window)
	{
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}
	glfwTerminate();
}

bool Window::ShouldClose() const
{
	return m_Window ? glfwWindowShouldClose(m_Window) : true;
}
