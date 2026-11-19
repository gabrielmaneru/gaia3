#include <gtest/gtest.h>

#include <core/types.h>
#include <math/mathHelpers.h>

TEST( MathHelpersTests, ToRadiansConvertsKnownAngles )
{
	EXPECT_FLOAT_EQ( ToRadians( 0.0f ), 0.0f );
	EXPECT_FLOAT_EQ( ToRadians( 180.0f ), 3.14159265f );
	EXPECT_FLOAT_EQ( ToRadians( -90.0f ), -1.57079633f );
}

TEST( MathHelpersTests, ToDegreesConvertsKnownAngles )
{
	EXPECT_FLOAT_EQ( ToDegrees( 0.0f ), 0.0f );
	EXPECT_FLOAT_EQ( ToDegrees( 3.14159265f ), 180.0f );
}

TEST( MathHelpersTests, AngleConversionsAreInverses )
{
	EXPECT_NEAR( ToDegrees( ToRadians( 37.5f ) ), 37.5f, 1e-4f );
}

TEST( MathHelpersTests, ClampLimitsToTheRange )
{
	EXPECT_FLOAT_EQ( Clamp( -5.0f, 0.0f, 1.0f ), 0.0f );
	EXPECT_FLOAT_EQ( Clamp( 5.0f, 0.0f, 1.0f ), 1.0f );
	EXPECT_FLOAT_EQ( Clamp( 0.25f, 0.0f, 1.0f ), 0.25f );
}

TEST( MathHelpersTests, ClampKeepsValuesOnTheBounds )
{
	EXPECT_FLOAT_EQ( Clamp( 0.0f, 0.0f, 1.0f ), 0.0f );
	EXPECT_FLOAT_EQ( Clamp( 1.0f, 0.0f, 1.0f ), 1.0f );
}

TEST( MathHelpersTests, RampMapsTheRangeToZeroOne )
{
	EXPECT_FLOAT_EQ( Ramp( 2.0f, 2.0f, 6.0f ), 0.0f );
	EXPECT_FLOAT_EQ( Ramp( 3.0f, 2.0f, 6.0f ), 0.25f );
	EXPECT_FLOAT_EQ( Ramp( 6.0f, 2.0f, 6.0f ), 1.0f );
}

TEST( MathHelpersTests, RampClampsOutsideTheRange )
{
	EXPECT_FLOAT_EQ( Ramp( -10.0f, 2.0f, 6.0f ), 0.0f );
	EXPECT_FLOAT_EQ( Ramp( 10.0f, 2.0f, 6.0f ), 1.0f );
}

TEST( MathHelpersTests, RampReturnsZeroForEmptyOrInvertedRanges )
{
	EXPECT_FLOAT_EQ( Ramp( 5.0f, 2.0f, 2.0f ), 0.0f );
	EXPECT_FLOAT_EQ( Ramp( 5.0f, 6.0f, 2.0f ), 0.0f );
}

TEST( MathHelpersTests, IsCloseChecksEveryComponent )
{
	const Vec3 a3( 1.0f, 2.0f, 3.0f );
	EXPECT_TRUE( IsClose( a3, Vec3( 1.05f, 1.95f, 3.05f ), 0.1f ) );
	EXPECT_FALSE( IsClose( a3, Vec3( 1.5f, 2.0f, 3.0f ), 0.1f ) );
	EXPECT_FALSE( IsClose( a3, Vec3( 1.0f, 2.5f, 3.0f ), 0.1f ) );
	EXPECT_FALSE( IsClose( a3, Vec3( 1.0f, 2.0f, 3.5f ), 0.1f ) );

	const Vec4 a4( 1.0f, 2.0f, 3.0f, 4.0f );
	EXPECT_TRUE( IsClose( a4, Vec4( 1.05f, 2.0f, 3.0f, 4.05f ), 0.1f ) );
	EXPECT_FALSE( IsClose( a4, Vec4( 1.0f, 2.0f, 3.0f, 4.5f ), 0.1f ) );
}

TEST( MathHelpersTests, MinAndMaxPickTheSmallerAndLarger )
{
	EXPECT_EQ( Min( 3, 7 ), 3 );
	EXPECT_EQ( Max( 3, 7 ), 7 );
	EXPECT_FLOAT_EQ( Min( -1.5f, -2.5f ), -2.5f );
	EXPECT_FLOAT_EQ( Max( -1.5f, -2.5f ), -1.5f );

	// Note: Mixed types name the type to compare in
	const u64 count = 12;
	const u32 limit = 8;
	EXPECT_EQ( Min<u64>( count, limit ), 8u );
}

TEST( MathHelpersTests, VectorMinAndMaxGoComponentByComponent )
{
	EXPECT_TRUE( Min( Vec2( 1.0f, 5.0f ), Vec2( 3.0f, 2.0f ) ) == Vec2( 1.0f, 2.0f ) );
	EXPECT_TRUE( Max( Vec2( 1.0f, 5.0f ), Vec2( 3.0f, 2.0f ) ) == Vec2( 3.0f, 5.0f ) );

	EXPECT_TRUE( Min( Vec3( 1.0f, -2.0f, 3.0f ), Vec3( 0.0f, 4.0f, 3.0f ) ) == Vec3( 0.0f, -2.0f, 3.0f ) );
	EXPECT_TRUE( Max( Vec3( 1.0f, -2.0f, 3.0f ), Vec3( 0.0f, 4.0f, 3.0f ) ) == Vec3( 1.0f, 4.0f, 3.0f ) );

	EXPECT_TRUE( Min( Vec4( 1.0f, 2.0f, 3.0f, 4.0f ), Vec4( 4.0f, 3.0f, 2.0f, 1.0f ) ) == Vec4( 1.0f, 2.0f, 2.0f, 1.0f ) );
	EXPECT_TRUE( Max( Vec4( 1.0f, 2.0f, 3.0f, 4.0f ), Vec4( 4.0f, 3.0f, 2.0f, 1.0f ) ) == Vec4( 4.0f, 3.0f, 3.0f, 4.0f ) );
}