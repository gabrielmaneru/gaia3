#include "testHelpers.h"

#include <gfx/objImporter.h>

#include <string>

namespace
{
struct ImportResult
{
	bool      bImported = false;
	ModelData data;
};

ImportResult ImportObjText( const std::string& text )
{
	ScopedTempDir directory;

	ImportResult result;
	result.bImported = ObjImporter::Import( directory.Write( "mesh.obj", text ), result.data );
	return result;
}

const char* const kTriangle = "v 0 0 0\nv 1 0 0\nv 0 1 0\n";
} // namespace

TEST( ObjImporterTests, MissingFileFails )
{
	ModelData data;

	EXPECT_FALSE( ObjImporter::Import( "this/file/does/not/exist.obj", data ) );
}

TEST( ObjImporterTests, LoadsTriangleWithAllAttributes )
{
	const ImportResult result = ImportObjText( R"(# comment
o triangle
mtllib unused.mtl
v 1 2 3
v 4 5 6
v 7 8 9
vt 0.25 0.75
vt 0.5
vn 0 0 1
usemtl unused
s off
f 1/1/1 2/2/1 3/1/1
)" );

	ASSERT_TRUE( result.bImported );
	const ModelData& data = result.data;

	ASSERT_EQ( data.positions.size(), 3u );
	ASSERT_EQ( data.normals.size(), 3u );
	ASSERT_EQ( data.uvs.size(), 3u );
	EXPECT_EQ( data.indices, ( std::vector<u32>{ 0, 1, 2 } ) );

	EXPECT_TRUE( data.positions[1] == Vec3( 4.0f, 5.0f, 6.0f ) );
	EXPECT_TRUE( data.normals[2] == Vec3( 0.0f, 0.0f, 1.0f ) );
	EXPECT_TRUE( data.uvs[0] == Vec2( 0.25f, 0.75f ) );
	EXPECT_TRUE( data.uvs[1] == Vec2( 0.5f, 0.0f ) );
}

TEST( ObjImporterTests, TriangulatesPolygonsAndSharesVertices )
{
	const ImportResult result = ImportObjText( "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.positions.size(), 4u );
	EXPECT_EQ( result.data.indices, ( std::vector<u32>{ 0, 1, 2, 0, 2, 3 } ) );
}

TEST( ObjImporterTests, SplitsVerticesWithDifferentAttributes )
{
	const ImportResult result = ImportObjText( "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nvn 0 0 -1\n"
	                                           "f 1//1 2//1 3//1\nf 1//2 3//2 2//2\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.positions.size(), 6u );
	EXPECT_EQ( result.data.normals.size(), 6u );
	EXPECT_EQ( result.data.indices.size(), 6u );
}

TEST( ObjImporterTests, MissingAttributesDefaultToZero )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "f 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.normals.size(), 3u );
	ASSERT_EQ( result.data.uvs.size(), 3u );
	EXPECT_TRUE( result.data.normals[0] == Vec3::V_Zero );
	EXPECT_TRUE( result.data.uvs[0] == Vec2::V_Zero );
}

TEST( ObjImporterTests, HandlesWindowsLineEndings )
{
	const ImportResult result = ImportObjText( "v 0 0 0\r\nv 1 0 0\r\nv 0 1 0\r\nusemtl stone\r\nf 1 2 3\r\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.indices.size(), 3u );
	ASSERT_EQ( result.data.meshes[0].subMeshes.size(), 1u );
	EXPECT_EQ( result.data.meshes[0].subMeshes[0].materialSlot, "stone" );
}

TEST( ObjImporterTests, RejectsMalformedData )
{
	EXPECT_FALSE( ImportObjText( "" ).bImported );
	EXPECT_FALSE( ImportObjText( kTriangle ).bImported );
	EXPECT_FALSE( ImportObjText( "v 0 0\n" ).bImported );
	EXPECT_FALSE( ImportObjText( std::string( kTriangle ) + "f 1 2\n" ).bImported );
	EXPECT_FALSE( ImportObjText( std::string( kTriangle ) + "f 1 2 4\n" ).bImported );
	EXPECT_FALSE( ImportObjText( std::string( kTriangle ) + "f 0 1 2\n" ).bImported );
	EXPECT_FALSE( ImportObjText( std::string( kTriangle ) + "f 1 2 x\n" ).bImported );
	EXPECT_FALSE( ImportObjText( std::string( kTriangle ) + "f 1/9 2 3\n" ).bImported );
}

TEST( ObjImporterTests, RejectsNegativeIndices )
{
	EXPECT_FALSE( ImportObjText( std::string( kTriangle ) + "f -3 -2 -1\n" ).bImported );
}

TEST( ObjImporterTests, FacesWithoutAMaterialFormOneSubMesh )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "f 1 2 3\nf 3 2 1\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes[0].subMeshes.size(), 1u );

	const SubMeshData& subMesh = result.data.meshes[0].subMeshes[0];
	EXPECT_EQ( subMesh.materialSlot, "" );
	EXPECT_EQ( subMesh.indexOffset, 0u );
	EXPECT_EQ( subMesh.indexCount, 6u );
}

TEST( ObjImporterTests, FacesAreGroupedByMaterialInTheOrderTheyAppear )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "usemtl bricks\nf 1 2 3\n"
	                                                                      "usemtl fabric\nf 3 2 1\n"
	                                                                      "usemtl bricks\nf 2 3 1\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes[0].subMeshes.size(), 2u );

	const SubMeshData& bricks = result.data.meshes[0].subMeshes[0];
	EXPECT_EQ( bricks.materialSlot, "bricks" );
	EXPECT_EQ( bricks.indexOffset, 0u );
	EXPECT_EQ( bricks.indexCount, 6u );

	const SubMeshData& fabric = result.data.meshes[0].subMeshes[1];
	EXPECT_EQ( fabric.materialSlot, "fabric" );
	EXPECT_EQ( fabric.indexOffset, 6u );
	EXPECT_EQ( fabric.indexCount, 3u );

	EXPECT_EQ( result.data.indices.size(), 9u );
}

TEST( ObjImporterTests, SubMeshesShareTheVertices )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "usemtl bricks\nf 1 2 3\nusemtl fabric\nf 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.positions.size(), 3u );
	EXPECT_EQ( result.data.indices, ( std::vector<u32>{ 0, 1, 2, 0, 1, 2 } ) );
}

TEST( ObjImporterTests, FacesBeforeTheFirstMaterialUseTheDefaultSlot )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "f 1 2 3\nusemtl bricks\nf 3 2 1\nusemtl\nf 2 3 1\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes[0].subMeshes.size(), 2u );
	EXPECT_EQ( result.data.meshes[0].subMeshes[0].materialSlot, "" );
	EXPECT_EQ( result.data.meshes[0].subMeshes[0].indexCount, 6u );
	EXPECT_EQ( result.data.meshes[0].subMeshes[1].materialSlot, "bricks" );
}

TEST( ObjImporterTests, MaterialsWithoutFacesAddNoSubMesh )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "usemtl unused\nusemtl bricks\nf 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes[0].subMeshes.size(), 1u );
	EXPECT_EQ( result.data.meshes[0].subMeshes[0].materialSlot, "bricks" );
}

TEST( ObjImporterTests, FacesWithoutAnObjectFormOneUnnamedMesh )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "f 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes.size(), 1u );
	EXPECT_EQ( result.data.meshes[0].name, "" );
}

TEST( ObjImporterTests, ObjectsAndGroupsBecomeMeshes )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "o column\nusemtl stone\nf 1 2 3\n"
	                                                                      "g curtain\nusemtl fabric\nf 3 2 1\nusemtl stone\nf 2 3 1\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes.size(), 2u );

	const MeshData& column = result.data.meshes[0];
	EXPECT_EQ( column.name, "column" );
	ASSERT_EQ( column.subMeshes.size(), 1u );
	EXPECT_EQ( column.subMeshes[0].materialSlot, "stone" );
	EXPECT_EQ( column.subMeshes[0].indexOffset, 0u );

	const MeshData& curtain = result.data.meshes[1];
	EXPECT_EQ( curtain.name, "curtain" );
	ASSERT_EQ( curtain.subMeshes.size(), 2u );
	EXPECT_EQ( curtain.subMeshes[0].materialSlot, "fabric" );
	EXPECT_EQ( curtain.subMeshes[0].indexOffset, 3u );
	EXPECT_EQ( curtain.subMeshes[1].materialSlot, "stone" );
	EXPECT_EQ( curtain.subMeshes[1].indexOffset, 6u );
}

TEST( ObjImporterTests, RepeatedObjectNamesJoinTheEarlierMesh )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "o a\nf 1 2 3\no b\nf 3 2 1\no a\nf 2 3 1\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes.size(), 2u );
	EXPECT_EQ( result.data.meshes[0].name, "a" );
	EXPECT_EQ( result.data.meshes[0].subMeshes[0].indexCount, 6u );
	EXPECT_EQ( result.data.meshes[1].subMeshes[0].indexCount, 3u );
}

TEST( ObjImporterTests, ObjectsWithoutFacesAddNoMesh )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "o empty\ng body\nf 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	ASSERT_EQ( result.data.meshes.size(), 1u );
	EXPECT_EQ( result.data.meshes[0].name, "body" );
}

TEST( ObjImporterTests, NamesKeepTheirSpaces )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "g wall left \r\nf 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.meshes[0].name, "wall left" );
}

TEST( ObjImporterTests, MaterialLibrariesAreKept )
{
	const ImportResult result = ImportObjText( "mtllib sponza.mtl extra.mtl\nmtllib more.mtl\n" + std::string( kTriangle ) + "f 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.materialLibraries, ( std::vector<std::string>{ "sponza.mtl", "extra.mtl", "more.mtl" } ) );
}

TEST( ObjImporterTests, MeshesShareTheVertices )
{
	const ImportResult result = ImportObjText( std::string( kTriangle ) + "o a\nf 1 2 3\no b\nf 1 2 3\n" );

	ASSERT_TRUE( result.bImported );
	EXPECT_EQ( result.data.positions.size(), 3u );
	EXPECT_EQ( result.data.indices.size(), 6u );
}

TEST( ObjImporterTests, ImportingAgainReplacesThePreviousData )
{
	ScopedTempDir     directory;
	const std::string path = directory.Write( "mesh.obj", std::string( kTriangle ) + "f 1 2 3\n" );

	ModelData data;
	ASSERT_TRUE( ObjImporter::Import( path, data ) );
	ASSERT_TRUE( ObjImporter::Import( path, data ) );

	EXPECT_EQ( data.positions.size(), 3u );
	EXPECT_EQ( data.meshes[0].subMeshes.size(), 1u );
}

TEST( ObjImporterTests, TheEngineCubeHasUVs )
{
	ModelData data;
	ASSERT_TRUE( ObjImporter::Import( GAIA3_ASSETS_PATH "models/cube.obj", data ) );

	EXPECT_EQ( data.indices.size(), 36u );
	ASSERT_EQ( data.meshes[0].subMeshes.size(), 1u );

	bool bHasUVs = false;
	for( const Vec2& uv : data.uvs )
	{
		bHasUVs = bHasUVs || uv.x != 0.0f || uv.y != 0.0f;
	}
	EXPECT_TRUE( bHasUVs );
}
