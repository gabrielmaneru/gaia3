#pragma once

typedef void* ProcAddressFn;

struct GLFWwindow;

class Window
{
public:
	bool Init( const char* title, rect canvas );
	void Update();
	void Shutdown();

	static ProcAddressFn GetProcAddressLoader();
	GLFWwindow*          GetWindowHandle() const { return m_WindowHandle; }

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
