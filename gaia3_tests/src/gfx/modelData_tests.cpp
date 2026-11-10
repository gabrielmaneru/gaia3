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
	for( const char* shape : { "cube", "tetrahedron", "octahedron", "icosahedron", "dodecahedron", "quad", "triangle" } )
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
