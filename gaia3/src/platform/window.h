#pragma once

#include <core/types.h>

using ProcAddressFn = void*;

struct GLFWwindow;

#include <string_view>

class Window
{
public:
	bool Init( std::string_view title, rect canvas );
	void Update();
	void Shutdown();

	static ProcAddressFn GetProcAddressLoader();
	GLFWwindow*          GetWindowHandle() const { return m_WindowHandle; }

	// Note: The operating system's display scale, for example 1.5 at 150%
	float GetContentScale() const;

	rect GetCanvas() const { return m_Canvas; }
	bool IsMinimized() const { return m_Canvas.x == 0 && m_Canvas.y == 0; }

	void SetVSyncEnabled( bool val );
	bool ShouldClose() const;

private:
	static void    OnWindowSizeChanged( GLFWwindow* window, int width, int height );
	static Window* s_Instance;

	GLFWwindow* m_WindowHandle    = nullptr;
	rect        m_Canvas          = {};
	bool        m_bIsVSyncEnabled = false;
};
