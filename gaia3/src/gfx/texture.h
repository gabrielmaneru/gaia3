#pragma once

#include <gfx/texture2D.h>
#include <system/resource.h>

#include <string_view>
#include <vector>

// Note: Pixels read from an image file, RGBA with one byte per channel, starting from the bottom row as OpenGL expects
struct TextureData
{
	u32             width  = 0;
	u32             height = 0;
	std::vector<u8> pixels;
};

// Note: A texture loaded from an image file in textures/
class Texture : public Resource, public Texture2D
{
	RTTI_DECLARE_RESOURCE( Texture )
public:
	static constexpr const char* kErrorName = "error";

	explicit Texture( std::string_view path, ResourceState state = ResourceState::NotLoaded );

	// Note: Returns the named texture, or the error texture with a warning so a missing one stands out
	static shared<Texture> FindOrError( std::string_view textureName, std::string_view requester );

	static bool ReadImage( std::string_view path, TextureData& outData );

	// Note: Optional import settings in a .meta file next to the image, like error.bmp.meta. They can set "filter"
	// (nearest or linear), "wrap" (repeat, mirror or clamp) and "mipmaps" (true or false), anything missing keeps its
	// value. Returns false, keeping the values, when the file has something it can't use
	static bool ReadImportSettings( std::string_view metaPath, TextureProperties& inOutProperties );

	bool LoadInternal() override;
	bool UnloadInternal() override;
};
