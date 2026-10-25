#pragma once

#include <core/window.h>

class Renderer
{
public:
	void Init(ProcAddressFn procAddressLoader);
	void Update();
	void Shutdown();

private:
	bool InitializeDummyShaders();
	bool InitializeDummyVertexBuffer();

	u32 m_VertexArray = 0;
	u32 m_VertexBuffer = 0;
	u32 m_Program = 0;
	bool m_IsInitialized = false;
};
