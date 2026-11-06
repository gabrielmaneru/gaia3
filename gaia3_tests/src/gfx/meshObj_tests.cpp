#include "testHelpers.h"

#include <gfx/meshObj.h>

#include <string>

namespace
{
MeshObj LoadObjText( const std::string& text )
{
	ScopedTempDir directory;
	return MeshObj( directory.Write( "mesh.obj", text ) );
}
} // namespace

TEST( MeshObjTests, MissingFileIsInvalid )
{
	const MeshObj mesh( "this/file/does/not/exist.obj" );
	EXPECT_FALSE( mesh.bIsValid );
}

TEST( MeshObjTests, LoadsTriangleWithAllAttributes )
{
	const MeshObj mesh = LoadObjText( R"(# comment
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

	ASSERT_TRUE( mesh.bIsValid );
	const VertexBufferObj& buffer = mesh.vertexBuffer;

	ASSERT_EQ( buffer.positions.size(), 3u );
	ASSERT_EQ( buffer.normals.size(), 3u );
	ASSERT_EQ( buffer.uvs.size(), 3u );
	EXPECT_EQ( mesh.indices, ( std::vector<u32>{ 0, 1, 2 } ) );

	EXPECT_TRUE( buffer.positions[1] == Vec3( 4.0f, 5.0f, 6.0f ) );
	EXPECT_TRUE( buffer.normals[2] == Vec3( 0.0f, 0.0f, 1.0f ) );
	EXPECT_TRUE( buffer.uvs[0] == Vec2( 0.25f, 0.75f ) );
	EXPECT_TRUE( buffer.uvs[1] == Vec2( 0.5f, 0.0f ) );
}

TEST( MeshObjTests, TriangulatesPolygonsAndSharesVertices )
{
	const MeshObj mesh = LoadObjText( "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n" );

	ASSERT_TRUE( mesh.bIsValid );
	EXPECT_EQ( mesh.vertexBuffer.positions.size(), 4u );
	EXPECT_EQ( mesh.indices, ( std::vector<u32>{ 0, 1, 2, 0, 2, 3 } ) );
}

TEST( MeshObjTests, SplitsVerticesWithDifferentAttributes )
{
	const MeshObj mesh = LoadObjText( "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nvn 0 0 -1\n"
	                                  "f 1//1 2//1 3//1\nf 1//2 3//2 2//2\n" );

	ASSERT_TRUE( mesh.bIsValid );
	EXPECT_EQ( mesh.vertexBuffer.positions.size(), 6u );
	EXPECT_EQ( mesh.vertexBuffer.normals.size(), 6u );
	EXPECT_EQ( mesh.indices.size(), 6u );
}

TEST( MeshObjTests, MissingAttributesDefaultToZero )
{
	const MeshObj mesh = LoadObjText( "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n" );

	ASSERT_TRUE( mesh.bIsValid );
	ASSERT_EQ( mesh.vertexBuffer.normals.size(), 3u );
	ASSERT_EQ( mesh.vertexBuffer.uvs.size(), 3u );
	EXPECT_TRUE( mesh.vertexBuffer.normals[0] == Vec3::V_Zero );
	EXPECT_TRUE( mesh.vertexBuffer.uvs[0] == Vec2::V_Zero );
}

TEST( MeshObjTests, HandlesWindowsLineEndings )
{
	const MeshObj mesh = LoadObjText( "v 0 0 0\r\nv 1 0 0\r\nv 0 1 0\r\nf 1 2 3\r\n" );

	ASSERT_TRUE( mesh.bIsValid );
	EXPECT_EQ( mesh.indices.size(), 3u );
}

TEST( MeshObjTests, RejectsMalformedData )
{
	const char* const kVertices = "v 0 0 0\nv 1 0 0\nv 0 1 0\n";

	EXPECT_FALSE( LoadObjText( "" ).bIsValid );
	EXPECT_FALSE( LoadObjText( kVertices ).bIsValid );
	EXPECT_FALSE( LoadObjText( "v 0 0\n" ).bIsValid );
	EXPECT_FALSE( LoadObjText( std::string( kVertices ) + "f 1 2\n" ).bIsValid );
	EXPECT_FALSE( LoadObjText( std::string( kVertices ) + "f 1 2 4\n" ).bIsValid );
	EXPECT_FALSE( LoadObjText( std::string( kVertices ) + "f 0 1 2\n" ).bIsValid );
	EXPECT_FALSE( LoadObjText( std::string( kVertices ) + "f 1 2 x\n" ).bIsValid );
	EXPECT_FALSE( LoadObjText( std::string( kVertices ) + "f 1/9 2 3\n" ).bIsValid );
}

TEST( MeshObjTests, RejectsNegativeIndices )
{
	const MeshObj mesh = LoadObjText( "v 0 0 0\nv 1 0 0\nv 0 1 0\nf -3 -2 -1\n" );

	EXPECT_FALSE( mesh.bIsValid );
}
