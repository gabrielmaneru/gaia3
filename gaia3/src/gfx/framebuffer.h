#pragma once

#include <core/types.h>
#include <gfx/texture2D.h>

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

	u32 GetColorTexture() const { return m_Color.GetId(); }

private:
	u32       m_Framebuffer = 0;
	Texture2D m_Color;
	Texture2D m_Depth;
};
