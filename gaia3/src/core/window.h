#pragma once

struct GLFWwindow;

class Window
{
public:
	void Init();
	void Update();
	void Shutdown();

	bool ShouldClose() const;

private:
	GLFWwindow* m_Window = nullptr;
};
