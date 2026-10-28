#pragma once

typedef void* ProcAddressFn;

struct GLFWwindow;

class Window
{
public:
	bool Init(const char* title, rect canvas);
	void Update();
	void Shutdown();

	static ProcAddressFn GetProcAddressLoader();
	GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

	void SetVSyncEnabled(bool val);
	bool ShouldClose() const;

private:
	GLFWwindow* m_WindowHandle = nullptr;
	bool m_bIsVSyncEnabled = false;
};
