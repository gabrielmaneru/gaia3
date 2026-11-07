#pragma once

#include <core/types.h>

// Note: Call Destroy while the OpenGL context is alive, the destructor does not release anything
class Framebuffer
{
public:
	bool Create( rect size );
	void Destroy();
	bool Resize( rect size );

	void        Bind() const;
	static void Unbind();

	void BlitToWindow( rect canvas ) const;

	u32  GetColorTexture() const { return m_ColorTexture; }
	rect GetSize() const { return m_Size; }

private:
	u32  m_Framebuffer  = 0;
	u32  m_ColorTexture = 0;
	u32  m_DepthBuffer  = 0;
	rect m_Size         = {};
};
