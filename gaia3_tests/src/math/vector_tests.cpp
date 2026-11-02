#include <gtest/gtest.h>

#include <math/mathHelpers.h>
#include <math/vector.h>
#include <math/vectorTransforms.h>

TEST( Vec3Tests, UnaryMinusAndEquality )
{
	const Vec3 a( 1.0f, -2.0f, 3.0f );
	const Vec3 b = -a;

	EXPECT_FLOAT_EQ( b.x, -1.0f );
	EXPECT_FLOAT_EQ( b.y, 2.0f );
	EXPECT_FLOAT_EQ( b.z, -3.0f );
	EXPECT_TRUE( a == Vec3( 1.0f, -2.0f, 3.0f ) );
	EXPECT_TRUE( a != b );
}

TEST( Vec3Tests, AddSubtractMultiply )
{
	const Vec3 a( 1.0f, 2.0f, 3.0f );
	const Vec3 b( 4.0f, 5.0f, 6.0f );

	const Vec3 sum = a + b;
	EXPECT_FLOAT_EQ( sum.x, 5.0f );
	EXPECT_FLOAT_EQ( sum.y, 7.0f );
	EXPECT_FLOAT_EQ( sum.z, 9.0f );

	const Vec3 diff = b - a;
	EXPECT_FLOAT_EQ( diff.x, 3.0f );
	EXPECT_FLOAT_EQ( diff.y, 3.0f );
	EXPECT_FLOAT_EQ( diff.z, 3.0f );

	const Vec3 scaled = a * Scalar( 2.5f );
	EXPECT_FLOAT_EQ( scaled.x, 2.5f );
	EXPECT_FLOAT_EQ( scaled.y, 5.0f );
	EXPECT_FLOAT_EQ( scaled.z, 7.5f );
}

TEST( Vec3Tests, ComponentWiseMultiplyAssignAndVec4Conversion )
{
	Vec3       a( 2.0f, 3.0f, 4.0f );
	const Vec3 b( 5.0f, 6.0f, 7.0f );

	const Vec3 multiplied = a * b;
	EXPECT_FLOAT_EQ( multiplied.x, 10.0f );
	EXPECT_FLOAT_EQ( multiplied.y, 18.0f );
	EXPECT_FLOAT_EQ( multiplied.z, 28.0f );

	a *= b;
	EXPECT_FLOAT_EQ( a.x, multiplied.x );
	EXPECT_FLOAT_EQ( a.y, multiplied.y );
	EXPECT_FLOAT_EQ( a.z, multiplied.z );

	const Vec3 fromVec4( Vec4( 9.0f, 8.0f, 7.0f, 6.0f ) );
	EXPECT_FLOAT_EQ( fromVec4.x, 9.0f );
	EXPECT_FLOAT_EQ( fromVec4.y, 8.0f );
	EXPECT_FLOAT_EQ( fromVec4.z, 7.0f );
}

TEST( Vec3Tests, NormalizeAndNormalized )
{
	Vec3       v( 3.0f, 0.0f, 4.0f );
	const Vec3 copy = v;
	const Vec3 n    = v.Normalized();

	EXPECT_NEAR( n.LengthSq(), 1.0f, 1e-5f );
	EXPECT_FLOAT_EQ( copy.x, v.x );
	EXPECT_FLOAT_EQ( copy.y, v.y );
	EXPECT_FLOAT_EQ( copy.z, v.z );

	v.Normalize();
	EXPECT_NEAR( v.LengthSq(), 1.0f, 1e-5f );

	Vec3 zero = Vec3::V_Zero;
	zero.Normalize();
	EXPECT_TRUE( zero == Vec3::V_Zero );
}

TEST( Vec4Tests, BasicOperationsAndNormalize )
{
	const Vec4 a( 1.0f, -2.0f, 3.0f, 4.0f );
	const Vec4 b( 2.0f, 3.0f, -1.0f, 0.5f );

	const Vec4 sum = a + b;
	EXPECT_FLOAT_EQ( sum.x, 3.0f );
	EXPECT_FLOAT_EQ( sum.y, 1.0f );
	EXPECT_FLOAT_EQ( sum.z, 2.0f );
	EXPECT_FLOAT_EQ( sum.w, 4.5f );

	Vec4       n( 0.0f, 3.0f, 4.0f, 0.0f );
	const Vec4 original = n;
	const Vec4 copyNorm = n.Normalized();
	EXPECT_NEAR( copyNorm.LengthSq(), 1.0f, 1e-5f );
	EXPECT_FLOAT_EQ( n.x, original.x );
	EXPECT_FLOAT_EQ( n.y, original.y );
	EXPECT_FLOAT_EQ( n.z, original.z );
	EXPECT_FLOAT_EQ( n.w, original.w );

	n.Normalize();
	EXPECT_NEAR( n.LengthSq(), 1.0f, 1e-5f );
}

TEST( Vec4Tests, EqualityUnaryAndComponentWiseMultiply )
{
	const Vec4 a( 1.0f, -2.0f, 3.0f, -4.0f );
	EXPECT_TRUE( a == Vec4( 1.0f, -2.0f, 3.0f, -4.0f ) );
	EXPECT_TRUE( a != Vec4( 1.0f, -2.0f, 3.0f, 4.0f ) );

	const Vec4 neg = -a;
	EXPECT_FLOAT_EQ( neg.x, -1.0f );
	EXPECT_FLOAT_EQ( neg.y, 2.0f );
	EXPECT_FLOAT_EQ( neg.z, -3.0f );
	EXPECT_FLOAT_EQ( neg.w, 4.0f );

	Vec4       lhs( 2.0f, 3.0f, 4.0f, 5.0f );
	const Vec4 rhs( 6.0f, 7.0f, 8.0f, 9.0f );

	const Vec4 multiplied = lhs * rhs;
	EXPECT_FLOAT_EQ( multiplied.x, 12.0f );
	EXPECT_FLOAT_EQ( multiplied.y, 21.0f );
	EXPECT_FLOAT_EQ( multiplied.z, 32.0f );
	EXPECT_FLOAT_EQ( multiplied.w, 45.0f );

	lhs *= rhs;
	EXPECT_FLOAT_EQ( lhs.x, multiplied.x );
	EXPECT_FLOAT_EQ( lhs.y, multiplied.y );
	EXPECT_FLOAT_EQ( lhs.z, multiplied.z );
	EXPECT_FLOAT_EQ( lhs.w, multiplied.w );

	const Vec4 fromVec3( Vec3( 2.0f, 4.0f, 6.0f ), 8.0f );
	EXPECT_FLOAT_EQ( fromVec3.x, 2.0f );
	EXPECT_FLOAT_EQ( fromVec3.y, 4.0f );
	EXPECT_FLOAT_EQ( fromVec3.z, 6.0f );
	EXPECT_FLOAT_EQ( fromVec3.w, 8.0f );
}

TEST( VectorTransformsTests, CrossAndDotVec3 )
{
	const Vec3 xAxis( 1.0f, 0.0f, 0.0f );
	const Vec3 yAxis( 0.0f, 1.0f, 0.0f );
	const Vec3 zAxis( 0.0f, 0.0f, 1.0f );

	const Vec3 cross = Cross( xAxis, yAxis );
	EXPECT_FLOAT_EQ( cross.x, zAxis.x );
	EXPECT_FLOAT_EQ( cross.y, zAxis.y );
	EXPECT_FLOAT_EQ( cross.z, zAxis.z );

	EXPECT_FLOAT_EQ( Dot( xAxis, yAxis ), 0.0f );
	EXPECT_FLOAT_EQ( Dot( xAxis, xAxis ), 1.0f );
}

TEST( VectorTransformsTests, CrossAndDotVec4 )
{
	const Vec4 a( 1.0f, 0.0f, 0.0f, 10.0f );
	const Vec4 b( 0.0f, 1.0f, 0.0f, 20.0f );

	const Vec4 cross = Cross( a, b );
	EXPECT_FLOAT_EQ( cross.x, 0.0f );
	EXPECT_FLOAT_EQ( cross.y, 0.0f );
	EXPECT_FLOAT_EQ( cross.z, 1.0f );
	EXPECT_FLOAT_EQ( cross.w, 0.0f );

	EXPECT_FLOAT_EQ( Dot( a, b ), 200.0f );
}
