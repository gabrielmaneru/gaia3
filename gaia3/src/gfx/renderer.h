#pragma once

#include <core/window.h>
#include "camera.h"

class Renderer
{
public:
	bool Init(ProcAddressFn procAddressLoader);
	void Update(rect canvas);
	void Shutdown();

private:
	bool InitializeDummyShaders();
	bool InitializeDummyVertexBuffer();

	Camera m_Camera;
	u32 m_VertexArray = 0;
	u32 m_VertexBuffer = 0;
	u32 m_Program = 0;
	int m_ViewMatrixLocation = -1;
	int m_ProjectionMatrixLocation = -1;
	bool m_IsInitialized = false;
};
