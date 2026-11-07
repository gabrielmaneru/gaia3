#pragma once

#include <core/types.h>
#include <gfx/framebuffer.h>
#include <gfx/rendererTypes.h>
#include <platform/window.h>

class Renderer
{
public:
	bool Init( ProcAddressFn procAddressLoader, rect canvas );
	void Update( rect renderSize );
	void PresentToWindow( rect canvas ) const;
	void Shutdown();

	const Framebuffer& GetRenderTarget() const { return m_RenderTarget; }

private:
	bool InitializeDummyShaders();

	Framebuffer m_RenderTarget;

	u32 m_Program                  = 0;
	int m_ModelMatrixLocation      = -1;
	int m_ViewMatrixLocation       = -1;
	int m_ProjectionMatrixLocation = -1;

	std::vector<Renderable*>   m_Renderables;
	std::vector<RenderCamera*> m_Cameras;

	friend class Renderable;
	friend class RenderCamera;
};
