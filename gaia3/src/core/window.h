#pragma once

typedef void* ProcAddressFn;

struct GLFWwindow;

class Window
{
public:
	void Init(const char* title, rect canvas);
	void Update();
	void Shutdown();
	ProcAddressFn GetProcAddressLoader() const;

	void SetVSyncEnabled(bool val);
	bool ShouldClose() const;

private:
	GLFWwindow* m_WindowHandle = nullptr;
	bool m_bIsVSyncEnabled = false;
};
