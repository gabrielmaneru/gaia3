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

	// Note: Supports 0 or 1 cameras, extra ones are ignored until the camera component can select a render target
	const RenderCamera* GetCamera() const { return m_Cameras.empty() ? nullptr : m_Cameras[0]; }

private:
	Framebuffer m_RenderTarget;

	std::vector<Renderable*>   m_Renderables;
	std::vector<RenderCamera*> m_Cameras;

	friend class Renderable;
	friend class RenderCamera;
};
