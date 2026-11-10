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

	const std::vector<RenderLight*>& GetLights() const { return m_Lights; }

	// Note: The lights handed to every draw, the first kMaxLights of them
	std::vector<LightParameters> GatherLights();

	DrawMode GetDrawMode() const { return m_DrawMode; }
	void     SetDrawMode( DrawMode mode ) { m_DrawMode = mode; }

	// Note: Works with every draw mode, so the cards' normals can be seen too
	bool IsAlphaTestEnabled() const { return m_bAlphaTest; }
	void SetAlphaTestEnabled( bool bEnabled ) { m_bAlphaTest = bEnabled; }

private:
	Framebuffer m_RenderTarget;
	DrawMode    m_DrawMode   = DrawMode::Lit;
	bool        m_bAlphaTest = true;

	std::vector<Renderable*>   m_Renderables;
	std::vector<RenderCamera*> m_Cameras;
	std::vector<RenderLight*>  m_Lights;

	bool m_bWarnedLightLimit = false;

	friend class Renderable;
	friend class RenderCamera;
	friend class RenderLight;
};
