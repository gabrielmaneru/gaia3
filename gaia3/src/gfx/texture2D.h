#pragma once

#include <core/types.h>

enum class TextureInternalFormat
{
	RGB8,
	RGBA8,
	RGB16F,
	RGBA16F,
	Depth24
};

enum class TexturePixelFormat
{
	RGB,
	RGBA,
	Depth
};

enum class TextureDataType
{
	UnsignedByte,
	UnsignedInt,
	Float
};

enum class TextureWrap
{
	Repeat,
	MirroredRepeat,
	ClampToEdge
};

enum class TextureFilter
{
	Nearest,
	Linear
};

// Note: How a texture stores its pixels on the GPU and how shaders sample it. internalFormat is the storage, while
// pixelFormat and dataType describe the pixels handed over when it is created
struct TextureProperties
{
	TextureInternalFormat internalFormat = TextureInternalFormat::RGBA8;
	TexturePixelFormat    pixelFormat    = TexturePixelFormat::RGBA;
	TextureDataType       dataType       = TextureDataType::UnsignedByte;
	TextureWrap           wrap           = TextureWrap::Repeat;
	TextureFilter         filter         = TextureFilter::Linear;
	bool                  bMipmaps       = true;

	static const TextureProperties V_DefaultColorRGBA;
	static const TextureProperties V_DefaultColorRGB;
	static const TextureProperties V_DefaultDepth;
	static const TextureProperties V_GBufferRGB;
	static const TextureProperties V_RenderTargetRGBA;
};

// Note: A 2D OpenGL texture and the properties it was created with, cube maps would be a separate class. Call Destroy
// while the OpenGL context is alive, the destructor does not release anything
class Texture2D
{
public:
	// Note: pixels can be null to create an empty texture, like a render target
	bool Create( rect size, const TextureProperties& properties, const void* pixels = nullptr );
	void Destroy();

	// Note: Keeps the properties but not the contents
	bool Resize( rect size );

	void Bind( u32 unit ) const;

	u32                      GetId() const { return m_Id; }
	rect                     GetSize() const { return m_Size; }
	const TextureProperties& GetProperties() const { return m_Properties; }

private:
	u32               m_Id   = 0;
	rect              m_Size = {};
	TextureProperties m_Properties;
};
