#include <gtest/gtest.h>

#include <math/mathHelpers.h>
#include <math/matrixTransforms.h>
#include <math/quaternion.h>
#include <math/quaternionTransforms.h>

#include <math.h>

TEST( QuatTests, UnaryMinus )
{
	const Quat q( 1.0f, -2.0f, 3.0f, -4.0f );
	const Quat n = -q;

	EXPECT_FLOAT_EQ( n.x, -1.0f );
	EXPECT_FLOAT_EQ( n.y, 2.0f );
	EXPECT_FLOAT_EQ( n.z, -3.0f );
	EXPECT_FLOAT_EQ( n.w, 4.0f );
}

TEST( QuatTests, MultiplyAndMultiplyAssignMatch )
{
	const Quat a( 0.25f, -0.5f, 0.75f, 1.0f );
	const Quat b( -1.0f, 2.0f, -3.0f, 0.5f );

	const Quat product = a * b;

	Quat assigned = a;
	assigned *= b;

	EXPECT_FLOAT_EQ( product.x, assigned.x );
	EXPECT_FLOAT_EQ( product.y, assigned.y );
	EXPECT_FLOAT_EQ( product.z, assigned.z );
	EXPECT_FLOAT_EQ( product.w, assigned.w );
}

TEST( QuatTests, ScalarMultiplyAndMultiplyAssignMatch )
{
	const Quat q( 1.0f, -2.0f, 3.5f, 4.0f );

	const Quat scaled = q * Scalar( 2.0f );
	EXPECT_FLOAT_EQ( scaled.x, 2.0f );
	EXPECT_FLOAT_EQ( scaled.y, -4.0f );
	EXPECT_FLOAT_EQ( scaled.z, 7.0f );
	EXPECT_FLOAT_EQ( scaled.w, 8.0f );

	Quat assigned = q;
	assigned *= Scalar( 2.0f );
	EXPECT_FLOAT_EQ( assigned.x, scaled.x );
	EXPECT_FLOAT_EQ( assigned.y, scaled.y );
	EXPECT_FLOAT_EQ( assigned.z, scaled.z );
	EXPECT_FLOAT_EQ( assigned.w, scaled.w );
}

TEST( QuatTests, LengthSqNormalizeAndNormalized )
{
	Quat q( 1.0f, 2.0f, 2.0f, 2.0f );
	EXPECT_FLOAT_EQ( q.LengthSq(), 13.0f );

	const Quat copy       = q;
	const Quat normalized = q.Normalized();
	EXPECT_NEAR( normalized.LengthSq(), 1.0f, 1e-5f );

	EXPECT_FLOAT_EQ( q.x, copy.x );
	EXPECT_FLOAT_EQ( q.y, copy.y );
	EXPECT_FLOAT_EQ( q.z, copy.z );
	EXPECT_FLOAT_EQ( q.w, copy.w );

	q.Normalize();
	EXPECT_NEAR( q.LengthSq(), 1.0f, 1e-5f );

	Quat zero = Quat::V_Zero;
	zero.Normalize();
	EXPECT_FLOAT_EQ( zero.x, Quat::V_Identity.x );
	EXPECT_FLOAT_EQ( zero.y, Quat::V_Identity.y );
	EXPECT_FLOAT_EQ( zero.z, Quat::V_Identity.z );
	EXPECT_FLOAT_EQ( zero.w, Quat::V_Identity.w );
}

TEST( QuatTransformsTests, DotComputesExpectedValue )
{
	const Quat a( 1.0f, 2.0f, 3.0f, 4.0f );
	const Quat b( 0.5f, -1.0f, 1.5f, 2.0f );

	EXPECT_FLOAT_EQ( Dot( a, b ), 11.0f );
}

TEST( QuatTransformsTests, QuatFromMatrix33IdentityIsUnitQuaternion )
{
	Matrix33 identity;
	identity.SetRow( 0, Vec3( 1.0f, 0.0f, 0.0f ) );
	identity.SetRow( 1, Vec3( 0.0f, 1.0f, 0.0f ) );
	identity.SetRow( 2, Vec3( 0.0f, 0.0f, 1.0f ) );

	const Quat q = QuatFromMatrix33( identity );

	EXPECT_NEAR( q.x, 0.0f, 1e-5f );
	EXPECT_NEAR( q.y, 0.0f, 1e-5f );
	EXPECT_NEAR( q.z, 0.0f, 1e-5f );
	EXPECT_NEAR( q.w, 1.0f, 1e-5f );
	EXPECT_NEAR( q.LengthSq(), 1.0f, 1e-5f );
}

TEST( QuatTests, RotateVec3AndVec4PreserveExpectedComponents )
{
	const float sinHalf = 0.70710678f;
	const float cosHalf = 0.70710678f;
	const Quat  q( 0.0f, sinHalf, 0.0f, cosHalf );

	const Vec3 rotatedVec3 = q.Rotate( Vec3::V_Front );
	EXPECT_NEAR( rotatedVec3.x, -1.0f, 1e-5f );
	EXPECT_NEAR( rotatedVec3.y, 0.0f, 1e-5f );
	EXPECT_NEAR( rotatedVec3.z, 0.0f, 1e-5f );

	const Vec4 rotatedVec4 = q.Rotate( Vec4( 1.0f, 0.0f, 0.0f, 3.0f ) );
	EXPECT_NEAR( rotatedVec4.x, 0.0f, 1e-5f );
	EXPECT_NEAR( rotatedVec4.y, 0.0f, 1e-5f );
	EXPECT_NEAR( rotatedVec4.z, -1.0f, 1e-5f );
	EXPECT_FLOAT_EQ( rotatedVec4.w, 3.0f );
}

TEST( QuatTransformsTests, LookAtQuatMatchesMatrixLookAtOrientation )
{
	const Vec3 forward( 0.3f, 0.4f, 0.8f );
	const Vec3 up( 0.0f, 1.0f, 0.2f );

	const Quat lookAtQuat = LookAtQuat( forward, up );
	const Quat fromMatrix = QuatFromMatrix33( LookAt33( forward, up ) );

	EXPECT_NEAR( lookAtQuat.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( fromMatrix.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( fabsf( Dot( lookAtQuat, fromMatrix ) ), 1.0f, 1e-5f );
}
