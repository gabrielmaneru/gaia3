#include "testHelpers.h"

#include <gfx/material.h>

namespace
{
struct ReadResult
{
	bool         bRead = false;
	MaterialData data;
};

ReadResult ReadMtlText( const std::string& text, std::string_view materialName = "" )
{
	ScopedTempDir directory;

	ReadResult result;
	result.bRead = Material::ReadMtl( directory.Write( "material.mtl", text ), materialName, result.data );
	return result;
}

const char* const kLibrary = "newmtl bricks\nKd 1 0 0\nmap_Kd bricks.png\n"
                             "newmtl fabric\nKd 0 1 0\nshader cloth\n"
                             "newmtl floor\nKd 0 0 1\n";
} // namespace

TEST( MaterialTests, ReadsColourTextureAndShader )
{
	const ReadResult result = ReadMtlText( "newmtl bricks\nNs 10\nKd 0.5 0.25 1\nmap_Kd bricks.png\nshader lit\n" );

	ASSERT_TRUE( result.bRead );
	EXPECT_EQ( result.data.name, "bricks" );
	EXPECT_TRUE( result.data.diffuseColor == Vec3( 0.5f, 0.25f, 1.0f ) );
	EXPECT_EQ( result.data.diffuseTexture, "bricks.png" );
	EXPECT_EQ( result.data.shader, "lit" );
}

TEST( MaterialTests, MissingLinesKeepTheDefaults )
{
	const ReadResult result = ReadMtlText( "# only a comment\nnewmtl plain\n" );

	ASSERT_TRUE( result.bRead );
	EXPECT_TRUE( result.data.diffuseColor == Vec3( 1.0f, 1.0f, 1.0f ) );
	EXPECT_TRUE( result.data.diffuseTexture.empty() );
	EXPECT_EQ( result.data.shader, "default" );
}

TEST( MaterialTests, TextureOptionsComeBeforeTheFileName )
{
	const ReadResult result = ReadMtlText( "newmtl bricks\nmap_Kd -s 2 2 1 -bm 0.5 textures/bricks.png\n" );

	ASSERT_TRUE( result.bRead );
	EXPECT_EQ( result.data.diffuseTexture, "textures/bricks.png" );
}

TEST( MaterialTests, ReadsTheNamedMaterialOfALibrary )
{
	const ReadResult result = ReadMtlText( kLibrary, "fabric" );

	ASSERT_TRUE( result.bRead );
	EXPECT_EQ( result.data.name, "fabric" );
	EXPECT_TRUE( result.data.diffuseColor == Vec3( 0.0f, 1.0f, 0.0f ) );
	EXPECT_TRUE( result.data.diffuseTexture.empty() );
	EXPECT_EQ( result.data.shader, "cloth" );
}

TEST( MaterialTests, ReadsTheLastMaterialOfALibrary )
{
	const ReadResult result = ReadMtlText( kLibrary, "floor" );

	ASSERT_TRUE( result.bRead );
	EXPECT_TRUE( result.data.diffuseColor == Vec3( 0.0f, 0.0f, 1.0f ) );
	EXPECT_EQ( result.data.shader, "default" );
}

TEST( MaterialTests, WithoutANameTheFirstMaterialIsRead )
{
	const ReadResult result = ReadMtlText( kLibrary );

	ASSERT_TRUE( result.bRead );
	EXPECT_EQ( result.data.name, "bricks" );
	EXPECT_EQ( result.data.diffuseTexture, "bricks.png" );
	EXPECT_TRUE( result.data.diffuseColor == Vec3( 1.0f, 0.0f, 0.0f ) );
}

TEST( MaterialTests, AMissingNameIsReported )
{
	LogCapture log;

	EXPECT_FALSE( ReadMtlText( kLibrary, "glass" ).bRead );
	EXPECT_TRUE( log.Contains( "has no material named glass" ) );
}

TEST( MaterialTests, ListsEveryMaterialOfALibrary )
{
	ScopedTempDir directory;

	EXPECT_EQ( Material::ListEntries( directory.Write( "sponza.mtl", kLibrary ) ), ( std::vector<std::string>{ "bricks", "fabric", "floor" } ) );
}

TEST( MaterialTests, RepeatedNamesAreListedOnceWithAWarning )
{
	ScopedTempDir directory;
	LogCapture    log;

	const std::vector<std::string> names = Material::ListEntries( directory.Write( "twice.mtl", "newmtl a\nnewmtl b\nnewmtl a\n" ) );

	EXPECT_EQ( names, ( std::vector<std::string>{ "a", "b" } ) );
	EXPECT_TRUE( log.Contains( "defines a twice" ) );
}

TEST( MaterialTests, FilesWithoutMaterialsListNothing )
{
	ScopedTempDir directory;

	EXPECT_TRUE( Material::ListEntries( directory.Write( "empty.mtl", "# nothing\n" ) ).empty() );
	EXPECT_TRUE( Material::ListEntries( "this/file/does/not/exist.mtl" ).empty() );
}

TEST( MaterialTests, InvalidColoursFail )
{
	LogCapture log;

	EXPECT_FALSE( ReadMtlText( "newmtl bad\nKd 1 x 0\n" ).bRead );
	EXPECT_TRUE( log.Contains( "error: Material: invalid colour at" ) );
}

TEST( MaterialTests, MissingFilesFail )
{
	MaterialData data;

	EXPECT_FALSE( Material::ReadMtl( "this/file/does/not/exist.mtl", "", data ) );
}

TEST( MaterialTests, HandlesWindowsLineEndings )
{
	ScopedTempDir     directory;
	const std::string path = directory.Write( "material.mtl", "newmtl bricks\r\nKd 0.5 0.5 0.5\r\nmap_Kd bricks.png\r\nnewmtl stone\r\n" );

	MaterialData data;
	ASSERT_TRUE( Material::ReadMtl( path, "bricks", data ) );
	EXPECT_EQ( data.diffuseTexture, "bricks.png" );
	EXPECT_EQ( Material::ListEntries( path ), ( std::vector<std::string>{ "bricks", "stone" } ) );
}

TEST( MaterialTests, MissingMaterialsFallBackToErrorWithAWarning )
{
	LogCapture log;

	// Note: There is no session here, so the error material is missing too
	EXPECT_EQ( Material::FindOrError( "test", "Model cube" ), nullptr );
	EXPECT_TRUE( log.Contains( "warning: Model cube: material test is not available, using error" ) );
	EXPECT_TRUE( log.Contains( "error: Model cube: the error material is not available either" ) );
}

TEST( MaterialTests, TheEngineMaterialsRead )
{
	MaterialData error;
	ASSERT_TRUE( Material::ReadMtl( GAIA3_ASSETS_PATH "materials/error.mtl", "error", error ) );
	EXPECT_EQ( error.diffuseTexture, "error.bmp" );
	EXPECT_EQ( error.shader, "default" );

	MaterialData fallback;
	ASSERT_TRUE( Material::ReadMtl( GAIA3_ASSETS_PATH "materials/default.mtl", "default", fallback ) );
	EXPECT_TRUE( fallback.diffuseTexture.empty() );
	EXPECT_EQ( fallback.shader, "default" );
}
