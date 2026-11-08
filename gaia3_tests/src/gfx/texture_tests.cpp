#include "testHelpers.h"

#include <gfx/texture.h>

#include <string>

namespace
{
// Note: A 24 bit BMP, one row with a red then a green pixel, padded to 4 bytes
std::string MakeTwoPixelBmp()
{
	const unsigned char bytes[] = {
	    'B', 'M', 62, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, // file header
	    40,
	    0,
	    0,
	    0,
	    2,
	    0,
	    0,
	    0,
	    1,
	    0,
	    0,
	    0,
	    1,
	    0,
	    24,
	    0,
	    0,
	    0,
	    0,
	    0,
	    8,
	    0,
	    0,
	    0, // info header
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0, // info header
	    0,
	    0,
	    255,
	    0,
	    255,
	    0,
	    0,
	    0 // pixels, blue green red
	};
	return std::string( reinterpret_cast<const char*>( bytes ), sizeof( bytes ) );
}

struct SettingsResult
{
	bool              bAllUsed = false;
	TextureProperties properties;
};

SettingsResult ReadSettingsText( const std::string& text )
{
	ScopedTempDir directory;

	SettingsResult result;
	result.bAllUsed = Texture::ReadImportSettings( directory.Write( "image.png.meta", text ), result.properties );
	return result;
}
} // namespace

TEST( TextureTests, ReadsABmpAsRgba )
{
	ScopedTempDir directory;

	TextureData image;
	ASSERT_TRUE( Texture::ReadImage( directory.Write( "pixels.bmp", MakeTwoPixelBmp() ), image ) );

	EXPECT_EQ( image.width, 2u );
	EXPECT_EQ( image.height, 1u );
	EXPECT_EQ( image.pixels, ( std::vector<u8>{ 255, 0, 0, 255, 0, 255, 0, 255 } ) );
}

TEST( TextureTests, ReadsATgaAsRgba )
{
	// Note: An uncompressed 24 bit TGA, like Sponza's textures, one row with a red then a green pixel
	const unsigned char bytes[] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 24, 0, // header
		0, 0, 255, 0, 255, 0                                  // pixels, blue green red
	};

	ScopedTempDir directory;
	TextureData   image;
	ASSERT_TRUE( Texture::ReadImage( directory.Write( "pixels.tga", std::string( reinterpret_cast<const char*>( bytes ), sizeof( bytes ) ) ), image ) );

	EXPECT_EQ( image.width, 2u );
	EXPECT_EQ( image.height, 1u );
	EXPECT_EQ( image.pixels, ( std::vector<u8>{ 255, 0, 0, 255, 0, 255, 0, 255 } ) );
}

TEST( TextureTests, ReadsTheEngineErrorTexture )
{
	TextureData image;
	ASSERT_TRUE( Texture::ReadImage( GAIA3_ASSETS_PATH "textures/error.bmp", image ) );

	EXPECT_EQ( image.width, 10u );
	EXPECT_EQ( image.height, 10u );
	EXPECT_EQ( image.pixels.size(), 10u * 10u * 4u );
}

TEST( TextureTests, MissingFilesAreReported )
{
	LogCapture log;
	TextureData  image;

	EXPECT_FALSE( Texture::ReadImage( "this/file/does/not/exist.png", image ) );
	EXPECT_TRUE( log.Contains( "error: Texture: failed to read" ) );
	EXPECT_TRUE( image.pixels.empty() );
}

TEST( TextureTests, FilesThatAreNotImagesFail )
{
	ScopedTempDir directory;
	TextureData     image;

	EXPECT_FALSE( Texture::ReadImage( directory.Write( "fake.png", "not an image" ), image ) );
}

TEST( TextureTests, MissingTexturesFallBackToErrorWithAWarning )
{
	LogCapture log;

	// Note: There is no session here, so the error texture is missing too
	EXPECT_EQ( Texture::FindOrError( "bricks.png", "Material wall" ), nullptr );
	EXPECT_TRUE( log.Contains( "warning: Material wall: texture bricks.png is not available, using error" ) );
	EXPECT_TRUE( log.Contains( "error: Material wall: the error texture is not available either, using the colour alone" ) );
}

TEST( TextureTests, WithoutImportSettingsThePropertiesStay )
{
	TextureProperties properties = TextureProperties::V_DefaultColorRGBA;

	EXPECT_TRUE( Texture::ReadImportSettings( "this/file/does/not/exist.png.meta", properties ) );
	EXPECT_EQ( properties.filter, TextureFilter::Linear );
	EXPECT_EQ( properties.wrap, TextureWrap::Repeat );
	EXPECT_TRUE( properties.bMipmaps );
}

TEST( TextureTests, ImportSettingsChangeTheSampling )
{
	const SettingsResult result = ReadSettingsText( R"({ "filter": "nearest", "wrap": "clamp", "mipmaps": false })" );

	ASSERT_TRUE( result.bAllUsed );
	EXPECT_EQ( result.properties.filter, TextureFilter::Nearest );
	EXPECT_EQ( result.properties.wrap, TextureWrap::ClampToEdge );
	EXPECT_FALSE( result.properties.bMipmaps );
	EXPECT_EQ( result.properties.internalFormat, TextureInternalFormat::RGBA8 );
}

TEST( TextureTests, MirrorWrapIsRead )
{
	EXPECT_EQ( ReadSettingsText( R"({ "wrap": "mirror" })" ).properties.wrap, TextureWrap::MirroredRepeat );
}

TEST( TextureTests, UnknownImportSettingsAreIgnoredWithAWarning )
{
	LogCapture           log;
	const SettingsResult result = ReadSettingsText( R"({ "filter": "blurry", "colour": "red", "wrap": "clamp" })" );

	EXPECT_FALSE( result.bAllUsed );
	EXPECT_EQ( result.properties.filter, TextureFilter::Linear );
	EXPECT_EQ( result.properties.wrap, TextureWrap::ClampToEdge );
	EXPECT_TRUE( log.Contains( "ignoring the filter setting" ) );
	EXPECT_TRUE( log.Contains( "ignoring the colour setting" ) );
}

TEST( TextureTests, ImportSettingsThatAreNotJsonAreIgnored )
{
	LogCapture log;

	EXPECT_FALSE( ReadSettingsText( "filter = nearest" ).bAllUsed );
	EXPECT_FALSE( ReadSettingsText( "[ 1, 2 ]" ).bAllUsed );
	EXPECT_TRUE( log.Contains( "it is not a JSON object" ) );
}

TEST( TextureTests, TheErrorTextureIsNotSmoothed )
{
	TextureProperties properties = TextureProperties::V_DefaultColorRGBA;

	EXPECT_TRUE( Texture::ReadImportSettings( GAIA3_ASSETS_PATH "textures/error.bmp.meta", properties ) );
	EXPECT_EQ( properties.filter, TextureFilter::Nearest );
}

TEST( TexturePropertiesTests, ColourPresetsRepeatAndUseMipmaps )
{
	const TextureProperties& rgba = TextureProperties::V_DefaultColorRGBA;
	EXPECT_EQ( rgba.internalFormat, TextureInternalFormat::RGBA8 );
	EXPECT_EQ( rgba.pixelFormat, TexturePixelFormat::RGBA );
	EXPECT_EQ( rgba.dataType, TextureDataType::UnsignedByte );
	EXPECT_EQ( rgba.wrap, TextureWrap::Repeat );
	EXPECT_EQ( rgba.filter, TextureFilter::Linear );
	EXPECT_TRUE( rgba.bMipmaps );

	const TextureProperties& rgb = TextureProperties::V_DefaultColorRGB;
	EXPECT_EQ( rgb.internalFormat, TextureInternalFormat::RGB8 );
	EXPECT_EQ( rgb.pixelFormat, TexturePixelFormat::RGB );
	EXPECT_TRUE( rgb.bMipmaps );
}

TEST( TexturePropertiesTests, RenderTargetPresetsClampWithoutMipmaps )
{
	const TextureProperties& depth = TextureProperties::V_DefaultDepth;
	EXPECT_EQ( depth.internalFormat, TextureInternalFormat::Depth24 );
	EXPECT_EQ( depth.pixelFormat, TexturePixelFormat::Depth );
	EXPECT_EQ( depth.wrap, TextureWrap::ClampToEdge );
	EXPECT_FALSE( depth.bMipmaps );

	const TextureProperties& gbuffer = TextureProperties::V_GBufferRGB;
	EXPECT_EQ( gbuffer.internalFormat, TextureInternalFormat::RGB16F );
	EXPECT_EQ( gbuffer.dataType, TextureDataType::Float );
	EXPECT_EQ( gbuffer.filter, TextureFilter::Nearest );
	EXPECT_FALSE( gbuffer.bMipmaps );

	const TextureProperties& target = TextureProperties::V_RenderTargetRGBA;
	EXPECT_EQ( target.internalFormat, TextureInternalFormat::RGBA8 );
	EXPECT_EQ( target.wrap, TextureWrap::ClampToEdge );
	EXPECT_FALSE( target.bMipmaps );
}

TEST( TexturePropertiesTests, AnEmptyTexture2DHasNoTexture )
{
	const Texture2D texture;

	EXPECT_EQ( texture.GetId(), 0u );
	EXPECT_EQ( texture.GetSize().x, 0 );
}
