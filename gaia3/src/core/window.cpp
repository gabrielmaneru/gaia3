#include "gaia3_pch.h"
#include "window.h"

#include <GLFW/glfw3.h>

Window* Window::s_Instance = nullptr;

bool Window::Init(const char* title, rect canvas)
{
	s_Instance = this;

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		m_WindowHandle = nullptr;
		return false;
	}

	// Request an OpenGL 3.3 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_WindowHandle = glfwCreateWindow(canvas.x, canvas.y, title, nullptr, nullptr);
	if (m_WindowHandle == nullptr)
	{
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(m_WindowHandle);
	glfwSetWindowSizeCallback(m_WindowHandle, &Window::OnWindowSizeChanged);

	int currentWidth = 0;
	int currentHeight = 0;
	glfwGetWindowSize(m_WindowHandle, &currentWidth, &currentHeight);
	m_Canvas.x = currentWidth > 0 ? currentWidth : 1;
	m_Canvas.y = currentHeight > 0 ? currentHeight : 1;

	SetVSyncEnabled(true);
	return true;
}

void Window::Update()
{
	if (m_WindowHandle == nullptr)
		return;

	// Present and poll events. If you're not using OpenGL, remove SwapBuffers.
	glfwSwapBuffers(m_WindowHandle);
	glfwPollEvents();
}

void Window::Shutdown()
{
	if (m_WindowHandle != nullptr)
	{
		glfwDestroyWindow(m_WindowHandle);
		m_WindowHandle = nullptr;
	}

	if (s_Instance == this)
		s_Instance = nullptr;

	glfwTerminate();
}

ProcAddressFn Window::GetProcAddressLoader()
{
	return reinterpret_cast<ProcAddressFn>(glfwGetProcAddress);
}

void Window::SetVSyncEnabled(bool val)
{
	if (val != m_bIsVSyncEnabled)
	{
		if (val)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		m_bIsVSyncEnabled = val;
	}
}

bool Window::ShouldClose() const
{
	return m_WindowHandle != nullptr ? glfwWindowShouldClose(m_WindowHandle) : true;
}

void Window::OnWindowSizeChanged(GLFWwindow* window, int width, int height)
{
	if (window == nullptr || s_Instance == nullptr)
		return;

	s_Instance->m_Canvas.x = width > 0 ? width : 1;
	s_Instance->m_Canvas.y = height > 0 ? height : 1;
}
