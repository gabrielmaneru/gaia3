#include "gaia3_pch.h"
#include "texture.h"

// Note: stb_image is header only, this is the one file that compiles it, limited to the formats Texture registers.
// It comes before core/app.h, whose input macro would rename stb's own variables called input
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_TGA
#include <stb_image.h>

#include <core/app.h>

#include <json/json.h>

#include <fstream>

RTTI_DEFINE_RESOURCE( Texture,
                      RTTI_RESOURCE_FOLDER( "textures" ),
                      RTTI_RESOURCE_EXTENSIONS( ".bmp", ".png", ".jpg", ".jpeg", ".tga" ) )

namespace
{
bool ReadFilter( const Json::Value& value, TextureFilter& outFilter )
{
	if( value == "nearest" )
		outFilter = TextureFilter::Nearest;
	else if( value == "linear" )
		outFilter = TextureFilter::Linear;
	else
		return false;

	return true;
}

bool ReadWrap( const Json::Value& value, TextureWrap& outWrap )
{
	if( value == "repeat" )
		outWrap = TextureWrap::Repeat;
	else if( value == "mirror" )
		outWrap = TextureWrap::MirroredRepeat;
	else if( value == "clamp" )
		outWrap = TextureWrap::ClampToEdge;
	else
		return false;

	return true;
}
} // namespace

Texture::Texture( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

shared<Texture> Texture::FindOrError( std::string_view textureName, std::string_view requester )
{
	ResourceManager& manager = App::GetSession().GetResourceManager();

	if( shared<Texture> texture = manager.FindResource<Texture>( textureName, true ) )
		return texture;

	GAIA_WARN( "{}: texture {} is not available, using {}", requester, textureName, kErrorName );

	shared<Texture> errorTexture = manager.FindResource<Texture>( kErrorName, true );
	if( errorTexture == nullptr )
	{
		GAIA_ERROR( "{}: the {} texture is not available either, using the colour alone", requester, kErrorName );
	}

	return errorTexture;
}

bool Texture::ReadImage( std::string_view path, TextureData& outData )
{
	outData = TextureData();

	stbi_set_flip_vertically_on_load( 1 );

	int width    = 0;
	int height   = 0;
	int channels = 0;

	stbi_uc* pixels = stbi_load( std::string( path ).c_str(), &width, &height, &channels, 4 );
	if( pixels == nullptr )
	{
		GAIA_ERROR( "Texture: failed to read {}: {}", path, stbi_failure_reason() );
		return false;
	}

	outData.width        = static_cast<u32>( width );
	outData.height       = static_cast<u32>( height );
	outData.fileChannels = static_cast<u32>( channels );
	outData.pixels.assign( pixels, pixels + static_cast<u64>( width ) * height * 4 );

	stbi_image_free( pixels );
	return true;
}

void TextureData::FlipGreen()
{
	for( u64 i = 1; i < pixels.size(); i += 4 )
		pixels[i] = static_cast<u8>( 255 - pixels[i] );
}

bool Texture::ReadImportSettings( std::string_view metaPath, TextureImportSettings& inOutSettings )
{
	// Note: Import settings are optional
	std::ifstream file{ std::string( metaPath ) };
	if( !file.is_open() )
		return true;

	Json::CharReaderBuilder readerBuilder;
	Json::Value             settings;
	std::string             parseErrors;
	if( !Json::parseFromStream( readerBuilder, file, &settings, &parseErrors ) || !settings.isObject() )
	{
		GAIA_WARN( "Texture: ignoring {}, it is not a JSON object", metaPath );
		return false;
	}

	bool bAllUsed = true;
	for( const std::string& settingName : settings.getMemberNames() )
	{
		const Json::Value& value = settings[settingName];

		bool bUsed = false;
		if( settingName == "filter" )
		{
			bUsed = ReadFilter( value, inOutSettings.properties.filter );
		}
		else if( settingName == "wrap" )
		{
			bUsed = ReadWrap( value, inOutSettings.properties.wrap );
		}
		else if( settingName == "mipmaps" && value.isBool() )
		{
			inOutSettings.properties.bMipmaps = value.asBool();
			bUsed                             = true;
		}
		else if( settingName == "flipGreen" && value.isBool() )
		{
			inOutSettings.bFlipGreen = value.asBool();
			bUsed                    = true;
		}

		if( !bUsed )
		{
			GAIA_WARN( "Texture: ignoring the {} setting in {}", settingName, metaPath );
			bAllUsed = false;
		}
	}

	return bAllUsed;
}

bool Texture::LoadInternal()
{
	TextureData image;
	if( !ReadImage( GetPath(), image ) )
		return false;

	TextureImportSettings settings;
	ReadImportSettings( std::string( GetPath() ) + ".meta", settings );

	if( settings.bFlipGreen )
		image.FlipGreen();

	// Note: stb_image counts grey with alpha as 2 channels and RGBA as 4
	m_bHasAlphaChannel = image.fileChannels == 2 || image.fileChannels == 4;

	if( !Create( rect{ static_cast<int>( image.width ), static_cast<int>( image.height ) }, settings.properties, image.pixels.data() ) )
		return false;

	GAIA_INFO( "Texture {}: {}x{}", GetName(), image.width, image.height );
	return true;
}

bool Texture::UnloadInternal()
{
	Destroy();
	return true;
}
