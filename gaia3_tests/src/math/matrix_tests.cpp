#include <gtest/gtest.h>

#include <math.h>

#include <math/mathHelpers.h>
#include <math/matrix.h>
#include <math/matrixTransforms.h>

TEST( MatrixTests, Matrix33DefaultIsZero )
{
	const Matrix33 m;

	EXPECT_FLOAT_EQ( m.col_0.x, 0.0f );
	EXPECT_FLOAT_EQ( m.col_0.y, 0.0f );
	EXPECT_FLOAT_EQ( m.col_0.z, 0.0f );
	EXPECT_FLOAT_EQ( m.col_1.x, 0.0f );
	EXPECT_FLOAT_EQ( m.col_1.y, 0.0f );
	EXPECT_FLOAT_EQ( m.col_1.z, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.x, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.y, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.z, 0.0f );
}

TEST( MatrixTests, Matrix44DefaultIsZero )
{
	const Matrix44 m;

	EXPECT_FLOAT_EQ( m.col_0.x, 0.0f );
	EXPECT_FLOAT_EQ( m.col_1.y, 0.0f );
	EXPECT_FLOAT_EQ( m.col_2.z, 0.0f );
	EXPECT_FLOAT_EQ( m.col_3.w, 0.0f );
}

TEST( MatrixTests, Matrix33SetRowWritesExpectedComponents )
{
	Matrix33 m;
	m.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	m.SetRow( 1, Vec3( 4.0f, 5.0f, 6.0f ) );
	m.SetRow( 2, Vec3( 7.0f, 8.0f, 9.0f ) );

	EXPECT_FLOAT_EQ( m.col_0.x, 1.0f );
	EXPECT_FLOAT_EQ( m.col_1.x, 2.0f );
	EXPECT_FLOAT_EQ( m.col_2.x, 3.0f );
	EXPECT_FLOAT_EQ( m.col_0.y, 4.0f );
	EXPECT_FLOAT_EQ( m.col_1.y, 5.0f );
	EXPECT_FLOAT_EQ( m.col_2.y, 6.0f );
	EXPECT_FLOAT_EQ( m.col_0.z, 7.0f );
	EXPECT_FLOAT_EQ( m.col_1.z, 8.0f );
	EXPECT_FLOAT_EQ( m.col_2.z, 9.0f );
}

TEST( MatrixTests, Matrix44SetRowWritesExpectedComponents )
{
	Matrix44 m;
	m.SetRow( 0, Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	m.SetRow( 1, Vec4( 5.0f, 6.0f, 7.0f, 8.0f ) );
	m.SetRow( 2, Vec4( 9.0f, 10.0f, 11.0f, 12.0f ) );
	m.SetRow( 3, Vec4( 13.0f, 14.0f, 15.0f, 16.0f ) );

	EXPECT_FLOAT_EQ( m.col_0.x, 1.0f );
	EXPECT_FLOAT_EQ( m.col_1.x, 2.0f );
	EXPECT_FLOAT_EQ( m.col_2.x, 3.0f );
	EXPECT_FLOAT_EQ( m.col_3.x, 4.0f );
	EXPECT_FLOAT_EQ( m.col_0.y, 5.0f );
	EXPECT_FLOAT_EQ( m.col_1.y, 6.0f );
	EXPECT_FLOAT_EQ( m.col_2.y, 7.0f );
	EXPECT_FLOAT_EQ( m.col_3.y, 8.0f );
	EXPECT_FLOAT_EQ( m.col_0.z, 9.0f );
	EXPECT_FLOAT_EQ( m.col_1.z, 10.0f );
	EXPECT_FLOAT_EQ( m.col_2.z, 11.0f );
	EXPECT_FLOAT_EQ( m.col_3.z, 12.0f );
	EXPECT_FLOAT_EQ( m.col_0.w, 13.0f );
	EXPECT_FLOAT_EQ( m.col_1.w, 14.0f );
	EXPECT_FLOAT_EQ( m.col_2.w, 15.0f );
	EXPECT_FLOAT_EQ( m.col_3.w, 16.0f );
}

TEST( MatrixTransformsTests, LookAtBuildsExpectedBasisAtOrigin )
{
	const Matrix44 view = LookAt( Vec3::V_Zero, Vec3::V_Front, Vec3::V_Up );

	EXPECT_FLOAT_EQ( view.col_0.x, 0.0f );
	EXPECT_FLOAT_EQ( view.col_0.y, 0.0f );
	EXPECT_FLOAT_EQ( view.col_0.z, -1.0f );

	EXPECT_FLOAT_EQ( view.col_1.x, 0.0f );
	EXPECT_FLOAT_EQ( view.col_1.y, 1.0f );
	EXPECT_FLOAT_EQ( view.col_1.z, 0.0f );

	EXPECT_FLOAT_EQ( view.col_2.x, 1.0f );
	EXPECT_FLOAT_EQ( view.col_2.y, 0.0f );
	EXPECT_FLOAT_EQ( view.col_2.z, 0.0f );

	EXPECT_FLOAT_EQ( view.col_3.x, 0.0f );
	EXPECT_FLOAT_EQ( view.col_3.y, 0.0f );
	EXPECT_FLOAT_EQ( view.col_3.z, 0.0f );
	EXPECT_FLOAT_EQ( view.col_3.w, 1.0f );
}

TEST( MatrixTransformsTests, LookAtComputesTranslationFromEye )
{
	const Vec3     eye( 2.0f, 3.0f, 4.0f );
	const Matrix44 view = LookAt( eye, Vec3::V_Front, Vec3::V_Up );

	EXPECT_FLOAT_EQ( view.col_3.x, -4.0f );
	EXPECT_FLOAT_EQ( view.col_3.y, -3.0f );
	EXPECT_FLOAT_EQ( view.col_3.z, 2.0f );
	EXPECT_FLOAT_EQ( view.col_3.w, 1.0f );
}

TEST( MatrixTransformsTests, PerspectiveProducesExpectedCoefficients )
{
	const float fov       = ToRadians( 60.0f );
	const float aspect    = 2.0f;
	const float nearPlane = 0.1f;
	const float farPlane  = 100.0f;

	const Matrix44 p = Perspective( fov, aspect, nearPlane, farPlane );

	const float tanHalfFovY = tanf( fov * 0.5f );
	const float expectedM00 = 1.0f / ( tanHalfFovY * aspect );
	const float expectedM11 = 1.0f / tanHalfFovY;
	const float expectedM22 = ( farPlane + nearPlane ) / ( nearPlane - farPlane );
	const float expectedM32 = ( 2.0f * farPlane * nearPlane ) / ( nearPlane - farPlane );

	EXPECT_NEAR( p.col_0.x, expectedM00, 1e-5f );
	EXPECT_NEAR( p.col_1.y, expectedM11, 1e-5f );
	EXPECT_NEAR( p.col_2.z, expectedM22, 1e-5f );
	EXPECT_NEAR( p.col_2.w, -1.0f, 1e-5f );
	EXPECT_NEAR( p.col_3.z, expectedM32, 1e-5f );
	EXPECT_NEAR( p.col_3.w, 0.0f, 1e-5f );
}

TEST( MatrixTransformsTests, PerspectiveClampsInvalidInputs )
{
	const float    fov = ToRadians( 70.0f );
	const Matrix44 p   = Perspective( fov, 0.0f, -1.0f, -2.0f );

	const float clampedAspect = 1.0f;
	const float clampedNear   = 0.01f;
	const float clampedFar    = 0.02f;
	const float tanHalfFovY   = tanf( fov * 0.5f );

	EXPECT_NEAR( p.col_0.x, 1.0f / ( tanHalfFovY * clampedAspect ), 1e-5f );
	EXPECT_NEAR( p.col_1.y, 1.0f / tanHalfFovY, 1e-5f );
	EXPECT_NEAR( p.col_2.z, ( clampedFar + clampedNear ) / ( clampedNear - clampedFar ), 1e-5f );
	EXPECT_NEAR( p.col_3.z, ( 2.0f * clampedFar * clampedNear ) / ( clampedNear - clampedFar ), 1e-5f );
}
