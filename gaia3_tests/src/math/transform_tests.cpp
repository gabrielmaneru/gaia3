#include <gtest/gtest.h>

#include <math/mathHelpers.h>
#include <math/transform.h>

TEST( TransformTests, MultiplyWithIdentityPreservesValues )
{
	const Transform t( Vec3( 1.0f, 2.0f, 3.0f ), Quat( 0.0f, 0.0f, 0.0f, 1.0f ), Vec3( 2.0f, 3.0f, 4.0f ) );
	const Transform r = t * Transform::V_Identity;

	EXPECT_FLOAT_EQ( r.position.x, t.position.x );
	EXPECT_FLOAT_EQ( r.position.y, t.position.y );
	EXPECT_FLOAT_EQ( r.position.z, t.position.z );
	EXPECT_FLOAT_EQ( r.rotation.x, t.rotation.x );
	EXPECT_FLOAT_EQ( r.rotation.y, t.rotation.y );
	EXPECT_FLOAT_EQ( r.rotation.z, t.rotation.z );
	EXPECT_FLOAT_EQ( r.rotation.w, t.rotation.w );
	EXPECT_FLOAT_EQ( r.scale.x, t.scale.x );
	EXPECT_FLOAT_EQ( r.scale.y, t.scale.y );
	EXPECT_FLOAT_EQ( r.scale.z, t.scale.z );
}

TEST( TransformTests, MultiplyOperatorDoesNotMutateLeftOperand )
{
	const Transform lhs( Vec3( 1.0f, 2.0f, 3.0f ), Quat::V_Identity, Vec3( 2.0f, 2.0f, 2.0f ) );
	const Transform rhs( Vec3( 4.0f, 5.0f, 6.0f ), Quat::V_Identity, Vec3( 3.0f, 3.0f, 3.0f ) );

	const Transform result = lhs * rhs;

	EXPECT_FLOAT_EQ( lhs.position.x, 1.0f );
	EXPECT_FLOAT_EQ( lhs.position.y, 2.0f );
	EXPECT_FLOAT_EQ( lhs.position.z, 3.0f );
	EXPECT_NE( result.position.x, lhs.position.x );
}

TEST( TransformTests, MultiplyComposesTranslationRotationAndScale )
{
	Transform       parent( Vec3( 1.0f, 2.0f, 3.0f ), Quat::V_Identity, Vec3( 2.0f, 3.0f, 4.0f ) );
	const Transform child( Vec3( 5.0f, 6.0f, 7.0f ), Quat::V_Identity, Vec3( 0.5f, 2.0f, 1.0f ) );

	parent *= child;

	EXPECT_FLOAT_EQ( parent.position.x, 11.0f );
	EXPECT_FLOAT_EQ( parent.position.y, 20.0f );
	EXPECT_FLOAT_EQ( parent.position.z, 31.0f );
	EXPECT_FLOAT_EQ( parent.rotation.x, 0.0f );
	EXPECT_FLOAT_EQ( parent.rotation.y, 0.0f );
	EXPECT_FLOAT_EQ( parent.rotation.z, 0.0f );
	EXPECT_FLOAT_EQ( parent.rotation.w, 1.0f );
	EXPECT_FLOAT_EQ( parent.scale.x, 1.0f );
	EXPECT_FLOAT_EQ( parent.scale.y, 6.0f );
	EXPECT_FLOAT_EQ( parent.scale.z, 4.0f );
}

TEST( TransformTests, ToMatrix44BuildsExpectedTRSForIdentityRotation )
{
	const Transform t( Vec3( 5.0f, 6.0f, 7.0f ), Quat::V_Identity, Vec3( 2.0f, 3.0f, 4.0f ) );
	const Matrix44  m = t.ToMatrix44();

	EXPECT_FLOAT_EQ( m.col_0.x, 2.0f );
	EXPECT_FLOAT_EQ( m.col_0.y, 0.0f );
	EXPECT_FLOAT_EQ( m.col_0.z, 0.0f );
	EXPECT_FLOAT_EQ( m.col_1.x, 0.0f );
	EXPECT_FLOAT_EQ( m.col_1.y, 3.0f );
	EXPECT_FLOAT_EQ( m.col_1.z, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.x, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.y, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.z, 4.0f );
	EXPECT_FLOAT_EQ( m.col_3.x, 5.0f );
	EXPECT_FLOAT_EQ( m.col_3.y, 6.0f );
	EXPECT_FLOAT_EQ( m.col_3.z, 7.0f );
	EXPECT_FLOAT_EQ( m.col_3.w, 1.0f );
}

TEST( TransformTests, ToMatrix44BuildsExpectedRotationBasis )
{
	const float     sinHalf = 0.70710678f;
	const float     cosHalf = 0.70710678f;
	const Transform t( Vec3::V_Zero, Quat( 0.0f, sinHalf, 0.0f, cosHalf ), Vec3::V_One );

	const Matrix44 m = t.ToMatrix44();

	EXPECT_NEAR( m.col_0.x, 0.0f, 1e-5f );
	EXPECT_NEAR( m.col_0.y, 0.0f, 1e-5f );
	EXPECT_NEAR( m.col_0.z, -1.0f, 1e-5f );
	EXPECT_NEAR( m.col_1.x, 0.0f, 1e-5f );
	EXPECT_NEAR( m.col_1.y, 1.0f, 1e-5f );
	EXPECT_NEAR( m.col_1.z, 0.0f, 1e-5f );
	EXPECT_NEAR( m.col_2.x, 1.0f, 1e-5f );
	EXPECT_NEAR( m.col_2.y, 0.0f, 1e-5f );
	EXPECT_NEAR( m.col_2.z, 0.0f, 1e-5f );
}

TEST( TransformTests, ToMatrix44NormalizesInputQuaternion )
{
	const float     sinHalf = 0.70710678f;
	const float     cosHalf = 0.70710678f;
	const Quat      nonUnit( 0.0f, sinHalf * 2.0f, 0.0f, cosHalf * 2.0f );
	const Transform t( Vec3::V_Zero, nonUnit, Vec3::V_One );

	const Matrix44 m = t.ToMatrix44();

	EXPECT_NEAR( m.col_0.x, 0.0f, 1e-5f );
	EXPECT_NEAR( m.col_0.z, -1.0f, 1e-5f );
	EXPECT_NEAR( m.col_2.x, 1.0f, 1e-5f );
	EXPECT_NEAR( m.col_2.z, 0.0f, 1e-5f );
}
