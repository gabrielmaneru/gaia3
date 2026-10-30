#include <gtest/gtest.h>

#include <math.h>

#include <math/mathHelpers.h>
#include <math/matrix.h>
#include <math/matrixTransforms.h>

TEST(MatrixTests, Matrix33DefaultIsZero)
{
	const Matrix33 m;

	EXPECT_FLOAT_EQ(m.col_0.x, 0.0f);
	EXPECT_FLOAT_EQ(m.col_0.y, 0.0f);
	EXPECT_FLOAT_EQ(m.col_0.z, 0.0f);
	EXPECT_FLOAT_EQ(m.col_1.x, 0.0f);
	EXPECT_FLOAT_EQ(m.col_1.y, 0.0f);
	EXPECT_FLOAT_EQ(m.col_1.z, 0.0f);
	EXPECT_FLOAT_EQ(m.col_2.x, 0.0f);
	EXPECT_FLOAT_EQ(m.col_2.y, 0.0f);
	EXPECT_FLOAT_EQ(m.col_2.z, 0.0f);
}

TEST(MatrixTests, Matrix44DefaultIsZero)
{
	const Matrix44 m;

	EXPECT_FLOAT_EQ(m.col_0.x, 0.0f);
	EXPECT_FLOAT_EQ(m.col_1.y, 0.0f);
	EXPECT_FLOAT_EQ(m.col_2.z, 0.0f);
	EXPECT_FLOAT_EQ(m.col_3.w, 0.0f);
}

TEST(MatrixTransformsTests, LookAtBuildsExpectedBasisAtOrigin)
{
	const Matrix44 view = LookAt(Vec3::V_Zero, Vec3::V_Front, Vec3::V_Up);

	EXPECT_FLOAT_EQ(view.col_0.x, 0.0f);
	EXPECT_FLOAT_EQ(view.col_0.y, 0.0f);
	EXPECT_FLOAT_EQ(view.col_0.z, 1.0f);

	EXPECT_FLOAT_EQ(view.col_1.x, 0.0f);
	EXPECT_FLOAT_EQ(view.col_1.y, 1.0f);
	EXPECT_FLOAT_EQ(view.col_1.z, 0.0f);

	EXPECT_FLOAT_EQ(view.col_2.x, -1.0f);
	EXPECT_FLOAT_EQ(view.col_2.y, 0.0f);
	EXPECT_FLOAT_EQ(view.col_2.z, 0.0f);

	EXPECT_FLOAT_EQ(view.col_3.x, 0.0f);
	EXPECT_FLOAT_EQ(view.col_3.y, 0.0f);
	EXPECT_FLOAT_EQ(view.col_3.z, 0.0f);
	EXPECT_FLOAT_EQ(view.col_3.w, 1.0f);
}

TEST(MatrixTransformsTests, LookAtComputesTranslationFromEye)
{
	const Vec3 eye(2.0f, 3.0f, 4.0f);
	const Matrix44 view = LookAt(eye, Vec3::V_Front, Vec3::V_Up);

	EXPECT_FLOAT_EQ(view.col_3.x, -4.0f);
	EXPECT_FLOAT_EQ(view.col_3.y, -3.0f);
	EXPECT_FLOAT_EQ(view.col_3.z, 2.0f);
	EXPECT_FLOAT_EQ(view.col_3.w, 1.0f);
}

TEST(MatrixTransformsTests, PerspectiveProducesExpectedCoefficients)
{
	const float fov = ToRadians(60.0f);
	const float aspect = 2.0f;
	const float nearPlane = 0.1f;
	const float farPlane = 100.0f;

	const Matrix44 p = Perspective(fov, aspect, nearPlane, farPlane);

	const float tanHalfFovY = tanf(fov * 0.5f);
	const float expectedM00 = 1.0f / (tanHalfFovY * aspect);
	const float expectedM11 = 1.0f / tanHalfFovY;
	const float expectedM22 = (farPlane + nearPlane) / (nearPlane - farPlane);
	const float expectedM32 = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);

	EXPECT_NEAR(p.col_0.x, expectedM00, 1e-5f);
	EXPECT_NEAR(p.col_1.y, expectedM11, 1e-5f);
	EXPECT_NEAR(p.col_2.z, expectedM22, 1e-5f);
	EXPECT_NEAR(p.col_2.w, -1.0f, 1e-5f);
	EXPECT_NEAR(p.col_3.z, expectedM32, 1e-5f);
	EXPECT_NEAR(p.col_3.w, 0.0f, 1e-5f);
}

TEST(MatrixTransformsTests, PerspectiveClampsInvalidInputs)
{
	const float fov = ToRadians(70.0f);
	const Matrix44 p = Perspective(fov, 0.0f, -1.0f, -2.0f);

	const float clampedAspect = 1.0f;
	const float clampedNear = 0.01f;
	const float clampedFar = 0.02f;
	const float tanHalfFovY = tanf(fov * 0.5f);

	EXPECT_NEAR(p.col_0.x, 1.0f / (tanHalfFovY * clampedAspect), 1e-5f);
	EXPECT_NEAR(p.col_1.y, 1.0f / tanHalfFovY, 1e-5f);
	EXPECT_NEAR(p.col_2.z, (clampedFar + clampedNear) / (clampedNear - clampedFar), 1e-5f);
	EXPECT_NEAR(p.col_3.z, (2.0f * clampedFar * clampedNear) / (clampedNear - clampedFar), 1e-5f);
}
