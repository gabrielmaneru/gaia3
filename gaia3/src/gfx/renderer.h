#pragma once

#include <core/types.h>
#include <gfx/rendererTypes.h>
#include <gfx/camera.h>
#include <platform/window.h>

class Renderer
{
public:
	bool Init( ProcAddressFn procAddressLoader, rect canvas );
	void Update( rect canvas );
	void Shutdown();

private:
	bool InitializeDummyShaders();
	bool InitializeDummyVertexBuffer();

	u32 m_VertexArray              = 0;
	u32 m_VertexBuffer             = 0;
	u32 m_Program                  = 0;
	int m_ViewMatrixLocation       = -1;
	int m_ProjectionMatrixLocation = -1;

	std::vector<Renderable*>   m_Renderables;
	std::vector<RenderCamera*> m_Cameras;

	friend class Renderable;
	friend class RenderCamera;
};
