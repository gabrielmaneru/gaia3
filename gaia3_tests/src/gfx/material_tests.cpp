#include "testHelpers.h"

#include <gfx/material.h>

namespace
{
struct ReadResult
{
	bool                      bRead = false;
	std::vector<MaterialData> materials;
};

ReadResult ReadMtlText( const std::string& text )
{
	ScopedTempDir directory;

	ReadResult result;
	result.bRead = Material::ReadMtl( directory.Write( "material.mtl", text ), result.materials );
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
	ASSERT_EQ( result.materials.size(), 1u );

	const MaterialData& bricks = result.materials[0];
	EXPECT_EQ( bricks.name, "bricks" );
	EXPECT_TRUE( bricks.diffuseColor == Vec3( 0.5f, 0.25f, 1.0f ) );
	EXPECT_EQ( bricks.diffuseTexture, "bricks.png" );
	EXPECT_EQ( bricks.shader, "lit" );
}

TEST( MaterialTests, MissingLinesKeepTheDefaults )
{
	const ReadResult result = ReadMtlText( "# only a comment\nnewmtl plain\n" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 1u );
	EXPECT_TRUE( result.materials[0].diffuseColor == Vec3( 1.0f, 1.0f, 1.0f ) );
	EXPECT_TRUE( result.materials[0].diffuseTexture.empty() );
	EXPECT_EQ( result.materials[0].shader, "default" );
}

TEST( MaterialTests, TextureOptionsComeBeforeTheFileName )
{
	const ReadResult result = ReadMtlText( "newmtl bricks\nmap_Kd -s 2 2 1 -bm 0.5 textures/bricks.png\n" );

	ASSERT_TRUE( result.bRead );
	EXPECT_EQ( result.materials[0].diffuseTexture, "textures/bricks.png" );
}

TEST( MaterialTests, ReadsTheOpacityMask )
{
	const ReadResult result = ReadMtlText( "newmtl leaf\nmap_Kd leaf.tga\nmap_d -imfchan l textures/leaf_mask.tga\nnewmtl stone\nmap_Kd stone.tga\n" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 2u );
	EXPECT_EQ( result.materials[0].opacityTexture, "textures/leaf_mask.tga" );
	EXPECT_EQ( result.materials[0].diffuseTexture, "leaf.tga" );
	EXPECT_TRUE( result.materials[1].opacityTexture.empty() );
}

TEST( MaterialTests, ReadsNormalMapsFromEveryBumpKeyword )
{
	const ReadResult result = ReadMtlText( "newmtl a\nmap_Bump -bm 1.5 a_ddn.tga\nnewmtl b\nmap_bump b_ddn.tga\nnewmtl c\nbump c_ddn.tga\n"
	                                       "newmtl d\nnorm d_ddn.tga\nnewmtl e\nmap_Disp e_disp.tga\n" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 5u );
	EXPECT_EQ( result.materials[0].normalTexture, "a_ddn.tga" );
	EXPECT_EQ( result.materials[1].normalTexture, "b_ddn.tga" );
	EXPECT_EQ( result.materials[2].normalTexture, "c_ddn.tga" );
	EXPECT_EQ( result.materials[3].normalTexture, "d_ddn.tga" );

	// Note: map_Disp is a displacement map, it moves the surface rather than bending its lighting
	EXPECT_TRUE( result.materials[4].normalTexture.empty() );
}

TEST( MaterialTests, ReadsTheTwoSidedLine )
{
	const ReadResult result = ReadMtlText( "newmtl leaf\nmap_Kd leaf.tga\ntwo_sided\nnewmtl stone\nmap_Kd stone.tga\n" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 2u );
	EXPECT_TRUE( result.materials[0].bTwoSided );

	// Note: Materials are one-sided unless they say otherwise
	EXPECT_FALSE( result.materials[1].bTwoSided );
}

TEST( MaterialTests, OnlyMaterialsWithAMaskAreAlphaTested )
{
	MaterialEntry leaf;
	leaf.data.diffuseTexture = "leaf.tga";
	leaf.data.opacityTexture = "leaf.tga";

	MaterialEntry stone;
	stone.data.diffuseTexture = "stone.tga";

	EXPECT_TRUE( leaf.IsAlphaTested() );
	EXPECT_FALSE( stone.IsAlphaTested() );
}

TEST( MaterialTests, DissolveValuesAreIgnored )
{
	// Note: Exporters often write d 0 for opaque materials, like Sponza does, so only map_d turns on cut-outs
	const ReadResult result = ReadMtlText( "newmtl glass\nd 0.000000\nTr 1.0\n" );

	ASSERT_TRUE( result.bRead );
	EXPECT_TRUE( result.materials[0].opacityTexture.empty() );
}

TEST( MaterialTests, ReadsEveryMaterialOfALibraryInOrder )
{
	const ReadResult result = ReadMtlText( kLibrary );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 3u );

	EXPECT_EQ( result.materials[0].name, "bricks" );
	EXPECT_EQ( result.materials[0].diffuseTexture, "bricks.png" );
	EXPECT_TRUE( result.materials[0].diffuseColor == Vec3( 1.0f, 0.0f, 0.0f ) );

	EXPECT_EQ( result.materials[1].name, "fabric" );
	EXPECT_TRUE( result.materials[1].diffuseTexture.empty() );
	EXPECT_EQ( result.materials[1].shader, "cloth" );

	EXPECT_EQ( result.materials[2].name, "floor" );
	EXPECT_TRUE( result.materials[2].diffuseColor == Vec3( 0.0f, 0.0f, 1.0f ) );
	EXPECT_EQ( result.materials[2].shader, "default" );
}

TEST( MaterialTests, AFileWithoutNewmtlIsOneUnnamedMaterial )
{
	const ReadResult result = ReadMtlText( "Kd 0.5 0.5 0.5\nmap_Kd stone.png\n" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 1u );
	EXPECT_EQ( result.materials[0].name, "" );
	EXPECT_EQ( result.materials[0].diffuseTexture, "stone.png" );
}

TEST( MaterialTests, AnEmptyFileIsOneDefaultMaterial )
{
	const ReadResult result = ReadMtlText( "" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 1u );
	EXPECT_EQ( result.materials[0].shader, "default" );
}

TEST( MaterialTests, RepeatedNamesAreReported )
{
	LogCapture       log;
	const ReadResult result = ReadMtlText( "newmtl a\nKd 1 0 0\nnewmtl b\nnewmtl a\nKd 0 1 0\n" );

	ASSERT_TRUE( result.bRead );
	EXPECT_EQ( result.materials.size(), 3u );
	EXPECT_TRUE( log.Contains( "defines a twice, the first one is used" ) );
}

TEST( MaterialTests, InvalidColoursFail )
{
	LogCapture log;

	EXPECT_FALSE( ReadMtlText( "newmtl bad\nKd 1 x 0\n" ).bRead );
	EXPECT_TRUE( log.Contains( "error: Material: invalid colour at" ) );
}

TEST( MaterialTests, MissingFilesFail )
{
	std::vector<MaterialData> materials;

	EXPECT_FALSE( Material::ReadMtl( "this/file/does/not/exist.mtl", materials ) );
}

TEST( MaterialTests, HandlesWindowsLineEndings )
{
	const ReadResult result = ReadMtlText( "newmtl bricks\r\nKd 0.5 0.5 0.5\r\nmap_Kd bricks.png\r\nnewmtl stone\r\n" );

	ASSERT_TRUE( result.bRead );
	ASSERT_EQ( result.materials.size(), 2u );
	EXPECT_EQ( result.materials[0].name, "bricks" );
	EXPECT_EQ( result.materials[0].diffuseTexture, "bricks.png" );
	EXPECT_EQ( result.materials[1].name, "stone" );
}

TEST( MaterialTests, AnUnloadedLibraryHasNoMaterials )
{
	const Material library( "materials/sponza.mtl" );

	EXPECT_EQ( library.GetName(), "sponza" );
	EXPECT_TRUE( library.GetEntries().empty() );
	EXPECT_EQ( library.FindEntry( "" ), nullptr );
	EXPECT_EQ( library.FindEntry( "floor" ), nullptr );
}

TEST( MaterialTests, MissingLibrariesFallBackToErrorWithAWarning )
{
	LogCapture log;

	// Note: There is no session here, so the error library is missing too
	EXPECT_EQ( Material::FindOrError( "test", "ModelComponent" ), nullptr );
	EXPECT_TRUE( log.Contains( "warning: ModelComponent: material test is not available, using error" ) );
	EXPECT_TRUE( log.Contains( "error: ModelComponent: the error material is not available either" ) );
}

TEST( MaterialTests, TheErrorEntryIsReportedWhenItIsMissing )
{
	LogCapture log;

	// Note: There is no session here, so the error library can't be found
	EXPECT_EQ( Material::FindErrorEntry( "Model sponza" ), nullptr );
	EXPECT_TRUE( log.Contains( "error: Model sponza: the error material is not available" ) );
}

TEST( MaterialTests, TheEngineMaterialsRead )
{
	std::vector<MaterialData> error;
	ASSERT_TRUE( Material::ReadMtl( GAIA3_ASSETS_PATH "materials/error.mtl", error ) );
	ASSERT_EQ( error.size(), 1u );
	EXPECT_EQ( error[0].diffuseTexture, "error.bmp" );
	EXPECT_EQ( error[0].shader, "default" );

	std::vector<MaterialData> fallback;
	ASSERT_TRUE( Material::ReadMtl( GAIA3_ASSETS_PATH "materials/default.mtl", fallback ) );
	ASSERT_EQ( fallback.size(), 1u );
	EXPECT_TRUE( fallback[0].diffuseTexture.empty() );
}
