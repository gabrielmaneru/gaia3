#include <gtest/gtest.h>

#include <math/boundingBox.h>

#include <math.h>

TEST( VectorScalarTests, EveryComponentGetsTheScalar )
{
	EXPECT_TRUE( Vec2( Scalar( 2.5f ) ) == Vec2( 2.5f, 2.5f ) );
	EXPECT_TRUE( Vec3( Scalar( -1.0f ) ) == Vec3( -1.0f, -1.0f, -1.0f ) );
	EXPECT_TRUE( Vec4( Scalar( 0.25f ) ) == Vec4( 0.25f, 0.25f, 0.25f, 0.25f ) );
}

TEST( AABBTests, StartsEmpty )
{
	const AABB box;

	EXPECT_TRUE( box.IsEmpty() );

	// Note: Inside out, so any first point becomes both corners
	EXPECT_TRUE( box.min == Vec3( Scalar( FLT_MAX ) ) );
	EXPECT_TRUE( box.max == Vec3( Scalar( -FLT_MAX ) ) );
	EXPECT_TRUE( box.GetCenter() == Vec3::V_Zero );
	EXPECT_TRUE( box.GetSize() == Vec3::V_Zero );
	EXPECT_FLOAT_EQ( box.GetRadius(), 0.0f );
}

TEST( AABBTests, OnePointIsAnEmptySizedBox )
{
	AABB box;
	box.Add( Vec3( 1.0f, 2.0f, 3.0f ) );

	EXPECT_FALSE( box.IsEmpty() );
	EXPECT_TRUE( box.GetCenter() == Vec3( 1.0f, 2.0f, 3.0f ) );
	EXPECT_TRUE( box.GetSize() == Vec3::V_Zero );
}

TEST( AABBTests, GrowsAroundEveryPoint )
{
	AABB box;
	box.Add( Vec3( 1.0f, -2.0f, 0.0f ) );
	box.Add( Vec3( -3.0f, 4.0f, 2.0f ) );
	box.Add( Vec3( 0.0f, 0.0f, -2.0f ) );

	EXPECT_TRUE( box.min == Vec3( -3.0f, -2.0f, -2.0f ) );
	EXPECT_TRUE( box.max == Vec3( 1.0f, 4.0f, 2.0f ) );
	EXPECT_TRUE( box.GetCenter() == Vec3( -1.0f, 1.0f, 0.0f ) );
	EXPECT_TRUE( box.GetSize() == Vec3( 4.0f, 6.0f, 4.0f ) );

	// Note: Half the diagonal, sqrt( 16 + 36 + 16 ) / 2
	EXPECT_NEAR( box.GetRadius(), sqrtf( 68.0f ) * 0.5f, 1e-5f );
}

TEST( AABBTests, AddingBoxesSkipsEmptyOnes )
{
	AABB first;
	first.Add( Vec3( 0.0f, 0.0f, 0.0f ) );
	first.Add( Vec3( 1.0f, 1.0f, 1.0f ) );

	AABB second;
	second.Add( Vec3( 2.0f, -1.0f, 0.5f ) );

	AABB merged;
	merged.Add( AABB() );
	EXPECT_TRUE( merged.IsEmpty() );

	merged.Add( first );
	merged.Add( second );
	EXPECT_TRUE( merged.min == Vec3( 0.0f, -1.0f, 0.0f ) );
	EXPECT_TRUE( merged.max == Vec3( 2.0f, 1.0f, 1.0f ) );
}
