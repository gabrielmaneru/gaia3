#include "testHelpers.h"

#include <gfx/modelData.h>
#include <gfx/objImporter.h>
#include <math/vectorTransforms.h>

#include <math.h>

namespace
{
// Note: A unit square facing +Z, its corners given UVs by uvOf
template <typename UVFunction>
ModelData MakeSquare( UVFunction uvOf )
{
	ModelData data;
	data.positions = { Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 1.0f, 1.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) };
	data.normals.assign( 4, Vec3( 0.0f, 0.0f, 1.0f ) );
	for( const Vec3& position : data.positions )
	{
		data.uvs.push_back( uvOf( position ) );
	}
	data.indices = { 0, 1, 2, 0, 2, 3 };
	return data;
}

void ExpectNear( const Vec4& actual, const Vec4& expected )
{
	EXPECT_NEAR( actual.x, expected.x, 1e-5f );
	EXPECT_NEAR( actual.y, expected.y, 1e-5f );
	EXPECT_NEAR( actual.z, expected.z, 1e-5f );
	EXPECT_FLOAT_EQ( actual.w, expected.w );
}
} // namespace

TEST( ModelDataTests, TangentsFollowTheTextureU )
{
	ModelData data = MakeSquare( []( const Vec3& p ) { return Vec2( p.x, p.y ); } );

	data.ComputeTangents();

	ASSERT_EQ( data.tangents.size(), data.positions.size() );
	for( const Vec4& tangent : data.tangents )
	{
		ExpectNear( tangent, Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	}
}

TEST( ModelDataTests, TangentsFollowARotatedTexture )
{
	// Note: U runs up the square and V runs to the left, a quarter turn
	ModelData data = MakeSquare( []( const Vec3& p ) { return Vec2( p.y, 1.0f - p.x ); } );

	data.ComputeTangents();

	for( const Vec4& tangent : data.tangents )
	{
		ExpectNear( tangent, Vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
	}
}

TEST( ModelDataTests, MirroredUVsFlipTheBitangent )
{
	// Note: U runs to the left, so V is on the other side of the tangent than cross( normal, tangent )
	ModelData data = MakeSquare( []( const Vec3& p ) { return Vec2( 1.0f - p.x, p.y ); } );

	data.ComputeTangents();

	for( const Vec4& tangent : data.tangents )
	{
		ExpectNear( tangent, Vec4( -1.0f, 0.0f, 0.0f, -1.0f ) );
	}
}

TEST( ModelDataTests, SquashedUVsStillGiveATangentAlongTheSurface )
{
	ModelData data = MakeSquare( []( const Vec3& ) { return Vec2( 0.5f, 0.5f ); } );

	data.ComputeTangents();

	for( const Vec4& tangent : data.tangents )
	{
		const Vec3 direction( tangent.x, tangent.y, tangent.z );
		EXPECT_NEAR( direction.LengthSq(), 1.0f, 1e-5f );
		EXPECT_NEAR( Dot( direction, Vec3( 0.0f, 0.0f, 1.0f ) ), 0.0f, 1e-5f );
	}
}

TEST( ModelDataTests, TheEngineShapesGetTangentsAlongTheirSurface )
{
	for( const char* shape : { "cube", "tetrahedron", "octahedron", "icosahedron", "dodecahedron", "quad", "triangle", "plane" } )
	{
		ModelData data;
		ASSERT_TRUE( ObjImporter::Import( std::string( GAIA3_ASSETS_PATH "models/" ) + shape + ".obj", data ) ) << shape;

		data.ComputeTangents();

		ASSERT_EQ( data.tangents.size(), data.positions.size() ) << shape;
		for( u64 i = 0; i < data.tangents.size(); ++i )
		{
			const Vec3 tangent( data.tangents[i].x, data.tangents[i].y, data.tangents[i].z );
			EXPECT_NEAR( tangent.LengthSq(), 1.0f, 1e-4f ) << shape;
			EXPECT_NEAR( Dot( tangent, data.normals[i].Normalized() ), 0.0f, 1e-4f ) << shape;
			EXPECT_FLOAT_EQ( fabsf( data.tangents[i].w ), 1.0f ) << shape;
		}
	}
}

TEST( ModelDataTests, TheEngineShapesFaceTheirNormals )
{
	// Note: Faces are counter-clockwise seen from the front, so each triangle's winding has to agree with its normals,
	// otherwise culling hides the lit side
	for( const char* shape : { "cube", "tetrahedron", "octahedron", "icosahedron", "dodecahedron", "quad", "triangle", "plane" } )
	{
		ModelData data;
		ASSERT_TRUE( ObjImporter::Import( std::string( GAIA3_ASSETS_PATH "models/" ) + shape + ".obj", data ) ) << shape;

		for( u64 i = 0; i + 2 < data.indices.size(); i += 3 )
		{
			const u32  a           = data.indices[i];
			const u32  b           = data.indices[i + 1];
			const u32  c           = data.indices[i + 2];
			const Vec3 faceNormal  = Cross( data.positions[b] - data.positions[a], data.positions[c] - data.positions[a] );
			const Vec3 storedTotal = data.normals[a] + data.normals[b] + data.normals[c];
			EXPECT_GT( Dot( faceNormal, storedTotal ), 0.0f ) << shape << " triangle " << i / 3;
		}
	}
}

TEST( ModelDataTests, FlatShapesFaceTheViewerAndThePlaneFacesUp )
{
	// Note: The camera looks down -Z, so flat shapes face +Z to be seen from its starting side, like glTF models
	for( const char* shape : { "quad", "triangle" } )
	{
		ModelData data;
		ASSERT_TRUE( ObjImporter::Import( std::string( GAIA3_ASSETS_PATH "models/" ) + shape + ".obj", data ) ) << shape;
		for( const Vec3& normal : data.normals )
		{
			EXPECT_TRUE( normal == Vec3( 0.0f, 0.0f, 1.0f ) ) << shape;
		}
	}

	ModelData plane;
	ASSERT_TRUE( ObjImporter::Import( GAIA3_ASSETS_PATH "models/plane.obj", plane ) );
	for( const Vec3& normal : plane.normals )
	{
		EXPECT_TRUE( normal == Vec3::V_Up );
	}

	// Note: One unit across, lying at y 0
	const AABB boundingVolume = plane.GetBoundingVolume();
	EXPECT_TRUE( boundingVolume.min == Vec3( -0.5f, 0.0f, -0.5f ) );
	EXPECT_TRUE( boundingVolume.max == Vec3( 0.5f, 0.0f, 0.5f ) );
}

namespace
{
// Note: Two meshes, the first with two sub-meshes, spread out so each one has its own box
ModelData MakeTwoMeshes()
{
	ModelData data;
	data.positions = {
	    Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ),
	    Vec3( 2.0f, 0.0f, 0.0f ), Vec3( 3.0f, 0.0f, 0.0f ), Vec3( 2.0f, 0.0f, 1.0f ),
	    Vec3( -5.0f, 4.0f, 2.0f ), Vec3( -4.0f, 4.0f, 2.0f ), Vec3( -5.0f, 6.0f, 3.0f ),
	    Vec3( 100.0f, 100.0f, 100.0f ), // Note: Not used by any triangle
	};
	data.indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	MeshData first;
	first.name      = "first";
	first.subMeshes = { SubMeshData{ 0, 3, "stone" }, SubMeshData{ 3, 3, "wood" } };

	MeshData second;
	second.name      = "second";
	second.subMeshes = { SubMeshData{ 6, 3, "metal" } };

	data.meshes = { first, second };
	return data;
}
} // namespace

TEST( ModelDataTests, BoundingVolumeCoversTheVerticesTrianglesUse )
{
	const ModelData data           = MakeTwoMeshes();
	const AABB      boundingVolume = data.GetBoundingVolume();

	// Note: The unused vertex at 100 is left out
	EXPECT_TRUE( boundingVolume.min == Vec3( -5.0f, 0.0f, 0.0f ) );
	EXPECT_TRUE( boundingVolume.max == Vec3( 3.0f, 6.0f, 3.0f ) );
	EXPECT_EQ( data.GetTriangleCount(), 3u );
}

TEST( ModelDataTests, MeshAndSubMeshBoundingVolumes )
{
	const ModelData data = MakeTwoMeshes();

	const AABB first = data.GetMeshBoundingVolume( 0 );
	EXPECT_TRUE( first.min == Vec3( 0.0f, 0.0f, 0.0f ) );
	EXPECT_TRUE( first.max == Vec3( 3.0f, 1.0f, 1.0f ) );

	const AABB wood = data.GetSubMeshBoundingVolume( 0, 1 );
	EXPECT_TRUE( wood.min == Vec3( 2.0f, 0.0f, 0.0f ) );
	EXPECT_TRUE( wood.max == Vec3( 3.0f, 0.0f, 1.0f ) );

	const AABB second = data.GetMeshBoundingVolume( 1 );
	EXPECT_TRUE( second.min == Vec3( -5.0f, 4.0f, 2.0f ) );
	EXPECT_TRUE( second.max == Vec3( -4.0f, 6.0f, 3.0f ) );
}

TEST( ModelDataTests, BoundingVolumesOutOfRangeAreEmpty )
{
	const ModelData data = MakeTwoMeshes();

	EXPECT_TRUE( data.GetMeshBoundingVolume( 2 ).IsEmpty() );
	EXPECT_TRUE( data.GetSubMeshBoundingVolume( 1, 1 ).IsEmpty() );
	EXPECT_TRUE( ModelData().GetBoundingVolume().IsEmpty() );

	// Note: A sub-mesh reaching past the indices only counts the ones that exist
	ModelData broken = data;
	broken.meshes[1].subMeshes[0].indexCount = 30;
	EXPECT_TRUE( broken.GetMeshBoundingVolume( 1 ).max == Vec3( -4.0f, 6.0f, 3.0f ) );
}
