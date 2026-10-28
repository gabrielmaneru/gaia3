#include "gaia3_pch.h"
#include "window.h"

#include <GLFW/glfw3.h>

bool Window::Init(const char* title, rect canvas)
{
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
