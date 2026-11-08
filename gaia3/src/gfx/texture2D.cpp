#include "gaia3_pch.h"
#include "texture2D.h"

#include <glad/glad.h>

const TextureProperties TextureProperties::V_DefaultColorRGBA = {};

const TextureProperties TextureProperties::V_DefaultColorRGB = {
    .internalFormat = TextureInternalFormat::RGB8,
    .pixelFormat    = TexturePixelFormat::RGB,
};

const TextureProperties TextureProperties::V_DefaultDepth = {
    .internalFormat = TextureInternalFormat::Depth24,
    .pixelFormat    = TexturePixelFormat::Depth,
    .dataType       = TextureDataType::UnsignedInt,
    .wrap           = TextureWrap::ClampToEdge,
    .filter         = TextureFilter::Nearest,
    .bMipmaps       = false,
};

const TextureProperties TextureProperties::V_GBufferRGB = {
    .internalFormat = TextureInternalFormat::RGB16F,
    .pixelFormat    = TexturePixelFormat::RGB,
    .dataType       = TextureDataType::Float,
    .wrap           = TextureWrap::ClampToEdge,
    .filter         = TextureFilter::Nearest,
    .bMipmaps       = false,
};

const TextureProperties TextureProperties::V_RenderTargetRGBA = {
    .wrap     = TextureWrap::ClampToEdge,
    .bMipmaps = false,
};

namespace
{
int ToGL( TextureInternalFormat format )
{
	switch( format )
	{
	case TextureInternalFormat::RGB8:
		return GL_RGB8;
	case TextureInternalFormat::RGBA8:
		return GL_RGBA8;
	case TextureInternalFormat::RGB16F:
		return GL_RGB16F;
	case TextureInternalFormat::RGBA16F:
		return GL_RGBA16F;
	case TextureInternalFormat::Depth24:
		return GL_DEPTH_COMPONENT24;
	}
	return GL_RGBA8;
}

u32 ToGL( TexturePixelFormat format )
{
	switch( format )
	{
	case TexturePixelFormat::RGB:
		return GL_RGB;
	case TexturePixelFormat::RGBA:
		return GL_RGBA;
	case TexturePixelFormat::Depth:
		return GL_DEPTH_COMPONENT;
	}
	return GL_RGBA;
}

u32 ToGL( TextureDataType type )
{
	switch( type )
	{
	case TextureDataType::UnsignedByte:
		return GL_UNSIGNED_BYTE;
	case TextureDataType::UnsignedInt:
		return GL_UNSIGNED_INT;
	case TextureDataType::Float:
		return GL_FLOAT;
	}
	return GL_UNSIGNED_BYTE;
}

int ToGL( TextureWrap wrap )
{
	switch( wrap )
	{
	case TextureWrap::Repeat:
		return GL_REPEAT;
	case TextureWrap::MirroredRepeat:
		return GL_MIRRORED_REPEAT;
	case TextureWrap::ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	}
	return GL_REPEAT;
}

int ToGLMinFilter( TextureFilter filter, bool bMipmaps )
{
	if( filter == TextureFilter::Nearest )
		return bMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

	return bMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
}

int ToGLMagFilter( TextureFilter filter )
{
	return filter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR;
}
} // namespace

bool Texture2D::Create( rect size, const TextureProperties& properties, const void* pixels )
{
	if( size.x <= 0 || size.y <= 0 )
	{
		GAIA_ERROR( "Texture2D: cannot create a {}x{} texture", size.x, size.y );
		return false;
	}

	Destroy();

	m_Size       = size;
	m_Properties = properties;

	glGenTextures( 1, &m_Id );
	glBindTexture( GL_TEXTURE_2D, m_Id );

	// Note: RGB rows aren't always a multiple of 4 bytes, so pixels are read without row padding
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, ToGL( properties.internalFormat ), size.x, size.y, 0, ToGL( properties.pixelFormat ), ToGL( properties.dataType ), pixels );

	if( properties.bMipmaps )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGL( properties.wrap ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGL( properties.wrap ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLMinFilter( properties.filter, properties.bMipmaps ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLMagFilter( properties.filter ) );

	glBindTexture( GL_TEXTURE_2D, 0 );
	return true;
}

void Texture2D::Destroy()
{
	if( m_Id != 0 )
	{
		glDeleteTextures( 1, &m_Id );
		m_Id = 0;
	}

	m_Size = {};
}

bool Texture2D::Resize( rect size )
{
	if( size.x == m_Size.x && size.y == m_Size.y )
		return true;

	const TextureProperties properties = m_Properties;
	return Create( size, properties );
}

void Texture2D::Bind( u32 unit ) const
{
	glActiveTexture( GL_TEXTURE0 + unit );
	glBindTexture( GL_TEXTURE_2D, m_Id );
}
