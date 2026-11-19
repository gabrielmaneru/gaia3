#include "modelOperations.h"

#include <gfx/modelData.h>
#include <gfx/objImporter.h>
#include <math/mathHelpers.h>
#include <math/matrixTransforms.h>
#include <math/quaternionTransforms.h>
#include <math/transform.h>
#include <math/vectorTransforms.h>

#include <gtest/gtest.h>

#include <math.h>
#include <string>
#include <vector>

namespace
{
// Note: Two triangles folded along the edge from vertex 1 to vertex 2 into a roof, the edge's vertices shared
ModelData MakeRoof()
{
	ModelData data;
	data.positions = { Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, -1.0f ), Vec3( 0.0f, 1.0f, 1.0f ), Vec3( 1.0f, 0.0f, 0.0f ) };
	data.normals.assign( 4, Vec3::V_Up );
	data.uvs.assign( 4, Vec2() );
	data.indices = { 0, 2, 1, 3, 1, 2 };
	data.meshes  = { MeshData{ "roof", { SubMeshData{ 0, 6, "tiles" } } } };
	return data;
}

// Note: Three meshes of one triangle each over their own vertices, the first two using two materials
ModelData MakeThreeMeshes()
{
	ModelData data;
	for( int i = 0; i < 4; ++i )
	{
		const float x = static_cast<float>( i ) * 10.0f;
		data.positions.insert( data.positions.end(), { Vec3( x, 0.0f, 0.0f ), Vec3( x + 1.0f, 0.0f, 0.0f ), Vec3( x, 1.0f, 0.0f ) } );
	}
	data.normals.assign( data.positions.size(), Vec3( 0.0f, 0.0f, 1.0f ) );
	data.uvs.assign( data.positions.size(), Vec2() );
	data.indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	data.meshes  = {
        MeshData{ "first", { SubMeshData{ 0, 3, "stone" }, SubMeshData{ 3, 3, "wood" } } },
        MeshData{ "second", { SubMeshData{ 6, 3, "stone" } } },
        MeshData{ "third", { SubMeshData{ 9, 3, "metal" } } },
    };
	return data;
}

// Note: The normal of a triangle from its winding, counter-clockwise seen from the front
Vec3 GetWindingNormal( const ModelData& data, u64 triangle )
{
	const Vec3& a = data.positions[data.indices[triangle * 3]];
	const Vec3& b = data.positions[data.indices[triangle * 3 + 1]];
	const Vec3& c = data.positions[data.indices[triangle * 3 + 2]];
	return Cross( b - a, c - a ).Normalized();
}

std::vector<std::string> GetMeshNames( const ModelData& data )
{
	std::vector<std::string> names;
	for( const MeshData& mesh : data.meshes )
	{
		names.push_back( mesh.name );
	}
	return names;
}

std::vector<std::string> GetSlots( const MeshData& mesh )
{
	std::vector<std::string> slots;
	for( const SubMeshData& subMesh : mesh.subMeshes )
	{
		slots.push_back( subMesh.materialSlot );
	}
	return slots;
}
} // namespace

TEST( ModelOperationsTests, TransformMovesTurnsAndScalesTheVertices )
{
	ModelData data = MakeRoof();
	data.tangents.assign( 4, Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

	const Transform transform( Vec3( 5.0f, 0.0f, 0.0f ), Quat(), Vec3( 2.0f, 2.0f, 2.0f ) );
	ModelOperations::Transform( data, transform.ToMatrix44() );

	EXPECT_TRUE( IsClose( data.positions[0], Vec3( 3.0f, 0.0f, 0.0f ), 1e-5f ) );
	EXPECT_TRUE( IsClose( data.positions[1], Vec3( 5.0f, 2.0f, -2.0f ), 1e-5f ) );

	// Note: Normals stay unit length, and tangents are worked out again later
	EXPECT_TRUE( IsClose( data.normals[0], Vec3::V_Up, 1e-5f ) );
	EXPECT_TRUE( data.tangents.empty() );
}

TEST( ModelOperationsTests, UnevenScaleKeepsNormalsAtRightAnglesToTheSurface )
{
	// Note: A slope facing up and along +X at 45 degrees, stretched twice as wide, becomes a gentler slope
	ModelData data;
	data.positions = { Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 0.0f, 0.0f, -1.0f ), Vec3( -1.0f, 1.0f, 0.0f ) };
	data.normals.assign( 3, Vec3( 1.0f, 1.0f, 0.0f ).Normalized() );
	data.indices = { 0, 1, 2 };

	ModelOperations::Transform( data, Transform( Vec3::V_Zero, Quat(), Vec3( 2.0f, 1.0f, 1.0f ) ).ToMatrix44() );

	const Vec3 surfaceNormal = GetWindingNormal( data, 0 );
	EXPECT_TRUE( IsClose( data.normals[0], surfaceNormal, 1e-5f ) );
	EXPECT_TRUE( IsClose( data.normals[0], Vec3( 0.5f, 1.0f, 0.0f ).Normalized(), 1e-5f ) );
}

TEST( ModelOperationsTests, MirroringKeepsFacesFacingOut )
{
	ModelData data;
	ASSERT_TRUE( ObjImporter::Import( GAIA3_ASSETS_PATH "models/cube.obj", data ) );

	ModelOperations::Transform( data, Transform( Vec3::V_Zero, Quat(), Vec3( -1.0f, 1.0f, 1.0f ) ).ToMatrix44() );

	// Note: The winding's front agrees with the stored normals, and both point away from the centre
	for( u64 t = 0; t < data.GetTriangleCount(); ++t )
	{
		const Vec3 windingNormal = GetWindingNormal( data, t );
		const Vec3 storedNormal  = data.normals[data.indices[t * 3]];
		EXPECT_GT( Dot( windingNormal, storedNormal ), 0.99f ) << "triangle " << t;
		EXPECT_GT( Dot( storedNormal, data.positions[data.indices[t * 3]] ), 0.0f ) << "triangle " << t;
	}
}

TEST( ModelOperationsTests, FlipFacesTurnsEveryFaceAround )
{
	ModelData data = MakeRoof();
	ModelOperations::ComputeNormals( data, 0.0f );
	const Vec3 before = GetWindingNormal( data, 0 );

	ModelOperations::FlipFaces( data );

	EXPECT_TRUE( IsClose( GetWindingNormal( data, 0 ), -before, 1e-5f ) );
	EXPECT_TRUE( IsClose( data.normals[data.indices[0]], -before, 1e-5f ) );
}

TEST( ModelOperationsTests, SharpEdgesSplitVerticesAndGentleOnesShareThem )
{
	// Note: The roof's faces meet at 90 degrees
	ModelData hard = MakeRoof();
	ModelOperations::ComputeNormals( hard, 60.0f );

	ASSERT_EQ( hard.positions.size(), 6u );
	EXPECT_TRUE( IsClose( hard.normals[hard.indices[0]], GetWindingNormal( hard, 0 ), 1e-5f ) );
	EXPECT_TRUE( IsClose( hard.normals[hard.indices[3]], GetWindingNormal( hard, 1 ), 1e-5f ) );

	ModelData smooth = MakeRoof();
	ModelOperations::ComputeNormals( smooth, 120.0f );

	// Note: The shared ridge points straight up, between the two faces
	ASSERT_EQ( smooth.positions.size(), 4u );
	EXPECT_TRUE( IsClose( smooth.normals[1], Vec3::V_Up, 1e-5f ) );
	EXPECT_TRUE( smooth.tangents.empty() );
}

TEST( ModelOperationsTests, ACubeTurnsSmoothOrStaysFlat )
{
	ModelData flat;
	ASSERT_TRUE( ObjImporter::Import( GAIA3_ASSETS_PATH "models/cube.obj", flat ) );
	ModelOperations::ComputeNormals( flat, 30.0f );
	for( u64 t = 0; t < flat.GetTriangleCount(); ++t )
	{
		EXPECT_TRUE( IsClose( flat.normals[flat.indices[t * 3]], GetWindingNormal( flat, t ), 1e-5f ) );
	}

	// Note: Fully smooth, every corner points away from the centre along the diagonal
	ModelData smooth;
	ASSERT_TRUE( ObjImporter::Import( GAIA3_ASSETS_PATH "models/cube.obj", smooth ) );
	ModelOperations::ComputeNormals( smooth, 180.0f );
	for( u64 i = 0; i < smooth.positions.size(); ++i )
	{
		EXPECT_TRUE( IsClose( smooth.normals[i], smooth.positions[i].Normalized(), 1e-5f ) ) << "vertex " << i;
	}
}

TEST( ModelOperationsTests, WeldingMergesOnlyMatchingVertices )
{
	ModelData data;
	data.positions = { Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 1.0f, 1.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ) };
	data.normals.assign( 6, Vec3( 0.0f, 0.0f, 1.0f ) );
	data.uvs     = { Vec2(), Vec2( 1.0f, 0.0f ), Vec2(), Vec2( 1.0f, 0.0f ), Vec2(), Vec2( 0.5f, 0.0f ) };
	data.indices = { 0, 1, 2, 3, 4, 2, 5, 4, 2 };

	ModelOperations::WeldVertices( data, 1e-4f );

	// Note: Vertex 3 matches vertex 1 and joins it, vertex 5 sits there too but has its own UV
	EXPECT_EQ( data.positions.size(), 5u );
	EXPECT_EQ( data.indices[3], data.indices[1] );
	EXPECT_NE( data.indices[6], data.indices[1] );
}

TEST( ModelOperationsTests, UnusedVerticesAreDroppedFromEveryStream )
{
	ModelData data;
	data.positions = { Vec3( 9.0f, 9.0f, 9.0f ), Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) };
	data.normals.assign( 4, Vec3( 0.0f, 0.0f, 1.0f ) );
	data.uvs      = { Vec2( 9.0f, 9.0f ), Vec2( 0.0f, 0.0f ), Vec2( 1.0f, 0.0f ), Vec2( 0.0f, 1.0f ) };
	data.tangents = { Vec4( 0.0f, 1.0f, 0.0f, 1.0f ), Vec4( 1.0f, 0.0f, 0.0f, 1.0f ), Vec4( 1.0f, 0.0f, 0.0f, 1.0f ), Vec4( 1.0f, 0.0f, 0.0f, -1.0f ) };
	data.indices  = { 1, 2, 3 };

	ModelOperations::RemoveUnusedVertices( data );

	EXPECT_EQ( data.positions.size(), 3u );
	EXPECT_EQ( data.uvs.size(), 3u );
	EXPECT_EQ( data.tangents.size(), 3u );
	EXPECT_EQ( data.indices, ( std::vector<u32>{ 0, 1, 2 } ) );
	EXPECT_TRUE( data.uvs[1] == Vec2( 1.0f, 0.0f ) );
	EXPECT_FLOAT_EQ( data.tangents[2].w, -1.0f );
}

TEST( ModelOperationsTests, RenamingChangesOnlyTheName )
{
	ModelData data = MakeThreeMeshes();

	EXPECT_TRUE( ModelOperations::RenameMesh( data, 1, "middle" ) );
	EXPECT_EQ( GetMeshNames( data ), ( std::vector<std::string>{ "first", "middle", "third" } ) );

	EXPECT_FALSE( ModelOperations::RenameMesh( data, 1, "middle" ) );
	EXPECT_FALSE( ModelOperations::RenameMesh( data, 5, "missing" ) );
}

TEST( ModelOperationsTests, DeletingAMeshDropsItsTrianglesAndVertices )
{
	ModelData data = MakeThreeMeshes();

	ASSERT_TRUE( ModelOperations::DeleteMesh( data, 0 ) );

	EXPECT_EQ( GetMeshNames( data ), ( std::vector<std::string>{ "second", "third" } ) );
	EXPECT_EQ( data.GetTriangleCount(), 2u );
	EXPECT_EQ( data.positions.size(), 6u );

	// Note: What remains still draws where it did
	EXPECT_TRUE( data.GetMeshBoundingVolume( 0 ).min == Vec3( 20.0f, 0.0f, 0.0f ) );
	EXPECT_TRUE( data.GetMeshBoundingVolume( 1 ).min == Vec3( 30.0f, 0.0f, 0.0f ) );
}

TEST( ModelOperationsTests, MergingJoinsSubMeshesOfTheSameMaterial )
{
	ModelData data = MakeThreeMeshes();

	ASSERT_TRUE( ModelOperations::MergeMeshes( data, 0, 1 ) );

	ASSERT_EQ( GetMeshNames( data ), ( std::vector<std::string>{ "first", "third" } ) );
	EXPECT_EQ( GetSlots( data.meshes[0] ), ( std::vector<std::string>{ "stone", "wood" } ) );
	EXPECT_EQ( data.meshes[0].subMeshes[0].indexCount, 6u );
	EXPECT_EQ( data.GetTriangleCount(), 4u );

	EXPECT_FALSE( ModelOperations::MergeMeshes( data, 0, 0 ) );
	EXPECT_FALSE( ModelOperations::MergeMeshes( data, 0, 9 ) );
}

TEST( ModelOperationsTests, SplittingMovesASubMeshIntoItsOwnMesh )
{
	ModelData data = MakeThreeMeshes();

	ASSERT_TRUE( ModelOperations::SplitSubMesh( data, 0, 1 ) );

	EXPECT_EQ( GetMeshNames( data ), ( std::vector<std::string>{ "first", "first_wood", "second", "third" } ) );
	EXPECT_EQ( GetSlots( data.meshes[1] ), ( std::vector<std::string>{ "wood" } ) );
	EXPECT_TRUE( data.GetMeshBoundingVolume( 1 ).min == Vec3( 10.0f, 0.0f, 0.0f ) );

	// Note: A mesh with one sub-mesh is already on its own
	EXPECT_FALSE( ModelOperations::SplitSubMesh( data, 1, 0 ) );
}

TEST( ModelOperationsTests, ChangingTheMaterialJoinsAnotherSubMeshUsingIt )
{
	ModelData data = MakeThreeMeshes();

	ASSERT_TRUE( ModelOperations::SetMaterialSlot( data, 0, 1, "stone" ) );
	EXPECT_EQ( GetSlots( data.meshes[0] ), ( std::vector<std::string>{ "stone" } ) );
	EXPECT_EQ( data.meshes[0].subMeshes[0].indexCount, 6u );

	ASSERT_TRUE( ModelOperations::SetMaterialSlot( data, 2, 0, "" ) );
	EXPECT_EQ( GetSlots( data.meshes[2] ), ( std::vector<std::string>{ "" } ) );

	EXPECT_FALSE( ModelOperations::SetMaterialSlot( data, 2, 0, "" ) );
	EXPECT_FALSE( ModelOperations::SetMaterialSlot( data, 0, 4, "wood" ) );
}

TEST( ModelOperationsTests, ImportedModelsKeepTheirLayoutThroughMeshEdits )
{
	// Note: Packing is the same grouping the importer makes, so an edit that changes nothing structural keeps the rest
	ModelData data;
	ASSERT_TRUE( ObjImporter::Import( GAIA3_ASSETS_PATH "models/cube.obj", data ) );
	const ModelData before = data;

	ASSERT_TRUE( ModelOperations::RenameMesh( data, 0, "box" ) );
	EXPECT_EQ( data.indices, before.indices );
	EXPECT_EQ( data.meshes[0].subMeshes.size(), before.meshes[0].subMeshes.size() );
}
