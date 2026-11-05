#include <gtest/gtest.h>

#include <math.h>

#include <math/mathHelpers.h>
#include <math/matrix.h>
#include <math/matrixTransforms.h>
#include <math/vector.h>
#include <math/vectorTransforms.h>

TEST( MatrixTests, Matrix33DefaultIsIdentity )
{
	const Matrix33 m;

	EXPECT_FLOAT_EQ( m.m00, 1.0f );
	EXPECT_FLOAT_EQ( m.m01, 0.0f );
	EXPECT_FLOAT_EQ( m.m02, 0.0f );
	EXPECT_FLOAT_EQ( m.m10, 0.0f );
	EXPECT_FLOAT_EQ( m.m11, 1.0f );
	EXPECT_FLOAT_EQ( m.m12, 0.0f );
	EXPECT_FLOAT_EQ( m.m20, 0.0f );
	EXPECT_FLOAT_EQ( m.m21, 0.0f );
	EXPECT_FLOAT_EQ( m.m22, 1.0f );
}

TEST( MatrixTests, Matrix44DefaultIsIdentity )
{
	const Matrix44 m;

	EXPECT_FLOAT_EQ( m.m00, 1.0f );
	EXPECT_FLOAT_EQ( m.m01, 0.0f );
	EXPECT_FLOAT_EQ( m.m02, 0.0f );
	EXPECT_FLOAT_EQ( m.m03, 0.0f );
	EXPECT_FLOAT_EQ( m.m10, 0.0f );
	EXPECT_FLOAT_EQ( m.m11, 1.0f );
	EXPECT_FLOAT_EQ( m.m12, 0.0f );
	EXPECT_FLOAT_EQ( m.m13, 0.0f );
	EXPECT_FLOAT_EQ( m.m20, 0.0f );
	EXPECT_FLOAT_EQ( m.m21, 0.0f );
	EXPECT_FLOAT_EQ( m.m22, 1.0f );
	EXPECT_FLOAT_EQ( m.m23, 0.0f );
	EXPECT_FLOAT_EQ( m.m30, 0.0f );
	EXPECT_FLOAT_EQ( m.m31, 0.0f );
	EXPECT_FLOAT_EQ( m.m32, 0.0f );
	EXPECT_FLOAT_EQ( m.m33, 1.0f );
}

TEST( MatrixTests, Matrix33SetRowWritesExpectedComponents )
{
	Matrix33 m;
	m.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	m.SetRow( 1, Vec3( 4.0f, 5.0f, 6.0f ) );
	m.SetRow( 2, Vec3( 7.0f, 8.0f, 9.0f ) );

	EXPECT_FLOAT_EQ( m.m00, 1.0f );
	EXPECT_FLOAT_EQ( m.m01, 2.0f );
	EXPECT_FLOAT_EQ( m.m02, 3.0f );
	EXPECT_FLOAT_EQ( m.m10, 4.0f );
	EXPECT_FLOAT_EQ( m.m11, 5.0f );
	EXPECT_FLOAT_EQ( m.m12, 6.0f );
	EXPECT_FLOAT_EQ( m.m20, 7.0f );
	EXPECT_FLOAT_EQ( m.m21, 8.0f );
	EXPECT_FLOAT_EQ( m.m22, 9.0f );
}

TEST( MatrixTests, Matrix33GetRowReturnsExpectedComponents )
{
	Matrix33 m;
	m.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	m.SetRow( 1, Vec3( 4.0f, 5.0f, 6.0f ) );
	m.SetRow( 2, Vec3( 7.0f, 8.0f, 9.0f ) );

	const Vec3 row0 = m.GetRow( 0 );
	const Vec3 row1 = m.GetRow( 1 );
	const Vec3 row2 = m.GetRow( 2 );

	EXPECT_FLOAT_EQ( row0.x, 1.0f );
	EXPECT_FLOAT_EQ( row0.y, 2.0f );
	EXPECT_FLOAT_EQ( row0.z, 3.0f );
	EXPECT_FLOAT_EQ( row1.x, 4.0f );
	EXPECT_FLOAT_EQ( row1.y, 5.0f );
	EXPECT_FLOAT_EQ( row1.z, 6.0f );
	EXPECT_FLOAT_EQ( row2.x, 7.0f );
	EXPECT_FLOAT_EQ( row2.y, 8.0f );
	EXPECT_FLOAT_EQ( row2.z, 9.0f );
}

TEST( MatrixTests, Matrix44SetRowWritesExpectedComponents )
{
	Matrix44 m;
	m.SetRow( 0, Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	m.SetRow( 1, Vec4( 5.0f, 6.0f, 7.0f, 8.0f ) );
	m.SetRow( 2, Vec4( 9.0f, 10.0f, 11.0f, 12.0f ) );
	m.SetRow( 3, Vec4( 13.0f, 14.0f, 15.0f, 16.0f ) );

	EXPECT_FLOAT_EQ( m.m00, 1.0f );
	EXPECT_FLOAT_EQ( m.m01, 2.0f );
	EXPECT_FLOAT_EQ( m.m02, 3.0f );
	EXPECT_FLOAT_EQ( m.m03, 4.0f );
	EXPECT_FLOAT_EQ( m.m10, 5.0f );
	EXPECT_FLOAT_EQ( m.m11, 6.0f );
	EXPECT_FLOAT_EQ( m.m12, 7.0f );
	EXPECT_FLOAT_EQ( m.m13, 8.0f );
	EXPECT_FLOAT_EQ( m.m20, 9.0f );
	EXPECT_FLOAT_EQ( m.m21, 10.0f );
	EXPECT_FLOAT_EQ( m.m22, 11.0f );
	EXPECT_FLOAT_EQ( m.m23, 12.0f );
	EXPECT_FLOAT_EQ( m.m30, 13.0f );
	EXPECT_FLOAT_EQ( m.m31, 14.0f );
	EXPECT_FLOAT_EQ( m.m32, 15.0f );
	EXPECT_FLOAT_EQ( m.m33, 16.0f );
}

TEST( MatrixTests, Matrix44GetRowReturnsExpectedComponents )
{
	Matrix44 m;
	m.SetRow( 0, Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	m.SetRow( 1, Vec4( 5.0f, 6.0f, 7.0f, 8.0f ) );
	m.SetRow( 2, Vec4( 9.0f, 10.0f, 11.0f, 12.0f ) );
	m.SetRow( 3, Vec4( 13.0f, 14.0f, 15.0f, 16.0f ) );

	const Vec4 row0 = m.GetRow( 0 );
	const Vec4 row1 = m.GetRow( 1 );
	const Vec4 row2 = m.GetRow( 2 );
	const Vec4 row3 = m.GetRow( 3 );

	EXPECT_FLOAT_EQ( row0.x, 1.0f );
	EXPECT_FLOAT_EQ( row0.y, 2.0f );
	EXPECT_FLOAT_EQ( row0.z, 3.0f );
	EXPECT_FLOAT_EQ( row0.w, 4.0f );
	EXPECT_FLOAT_EQ( row1.x, 5.0f );
	EXPECT_FLOAT_EQ( row1.y, 6.0f );
	EXPECT_FLOAT_EQ( row1.z, 7.0f );
	EXPECT_FLOAT_EQ( row1.w, 8.0f );
	EXPECT_FLOAT_EQ( row2.x, 9.0f );
	EXPECT_FLOAT_EQ( row2.y, 10.0f );
	EXPECT_FLOAT_EQ( row2.z, 11.0f );
	EXPECT_FLOAT_EQ( row2.w, 12.0f );
	EXPECT_FLOAT_EQ( row3.x, 13.0f );
	EXPECT_FLOAT_EQ( row3.y, 14.0f );
	EXPECT_FLOAT_EQ( row3.z, 15.0f );
	EXPECT_FLOAT_EQ( row3.w, 16.0f );
}

TEST( MatrixTests, Matrix44ConstructorFromMatrix33CopiesRotationAndIdentityLastColumn )
{
	Matrix33 m33;
	m33.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	m33.SetRow( 1, Vec3( 4.0f, 5.0f, 6.0f ) );
	m33.SetRow( 2, Vec3( 7.0f, 8.0f, 9.0f ) );

	const Matrix44 m44( m33 );

	EXPECT_FLOAT_EQ( m44.m00, 1.0f );
	EXPECT_FLOAT_EQ( m44.m01, 2.0f );
	EXPECT_FLOAT_EQ( m44.m02, 3.0f );
	EXPECT_FLOAT_EQ( m44.m10, 4.0f );
	EXPECT_FLOAT_EQ( m44.m11, 5.0f );
	EXPECT_FLOAT_EQ( m44.m12, 6.0f );
	EXPECT_FLOAT_EQ( m44.m20, 7.0f );
	EXPECT_FLOAT_EQ( m44.m21, 8.0f );
	EXPECT_FLOAT_EQ( m44.m22, 9.0f );

	EXPECT_FLOAT_EQ( m44.m30, 0.0f );
	EXPECT_FLOAT_EQ( m44.m31, 0.0f );
	EXPECT_FLOAT_EQ( m44.m32, 0.0f );
	EXPECT_FLOAT_EQ( m44.m03, 0.0f );
	EXPECT_FLOAT_EQ( m44.m13, 0.0f );
	EXPECT_FLOAT_EQ( m44.m23, 0.0f );
	EXPECT_FLOAT_EQ( m44.m33, 1.0f );
}

TEST( MatrixTests, Matrix33MultiplyProducesExpectedProduct )
{
	Matrix33 a;
	a.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	a.SetRow( 1, Vec3( 0.0f, 1.0f, 4.0f ) );
	a.SetRow( 2, Vec3( 5.0f, 6.0f, 0.0f ) );

	Matrix33 b;
	b.SetRow( 0, Vec3( -2.0f, 1.0f, 0.0f ) );
	b.SetRow( 1, Vec3( 3.0f, 0.0f, 1.0f ) );
	b.SetRow( 2, Vec3( 4.0f, -1.0f, 2.0f ) );

	const Matrix33 c = a * b;

	EXPECT_FLOAT_EQ( c.GetRow( 0 ).x, 16.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 0 ).y, -2.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 0 ).z, 8.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 1 ).x, 19.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 1 ).y, -4.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 1 ).z, 9.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 2 ).x, 8.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 2 ).y, 5.0f );
	EXPECT_FLOAT_EQ( c.GetRow( 2 ).z, 6.0f );

	Matrix33 inplace = a;
	inplace *= b;
	EXPECT_FLOAT_EQ( inplace.GetRow( 0 ).x, c.GetRow( 0 ).x );
	EXPECT_FLOAT_EQ( inplace.GetRow( 0 ).y, c.GetRow( 0 ).y );
	EXPECT_FLOAT_EQ( inplace.GetRow( 0 ).z, c.GetRow( 0 ).z );
	EXPECT_FLOAT_EQ( inplace.GetRow( 1 ).x, c.GetRow( 1 ).x );
	EXPECT_FLOAT_EQ( inplace.GetRow( 1 ).y, c.GetRow( 1 ).y );
	EXPECT_FLOAT_EQ( inplace.GetRow( 1 ).z, c.GetRow( 1 ).z );
	EXPECT_FLOAT_EQ( inplace.GetRow( 2 ).x, c.GetRow( 2 ).x );
	EXPECT_FLOAT_EQ( inplace.GetRow( 2 ).y, c.GetRow( 2 ).y );
	EXPECT_FLOAT_EQ( inplace.GetRow( 2 ).z, c.GetRow( 2 ).z );
}

TEST( MatrixTests, MatrixMultiplyWithIdentityPreservesMatrix )
{
	Matrix33 m33;
	m33.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	m33.SetRow( 1, Vec3( 4.0f, 5.0f, 6.0f ) );
	m33.SetRow( 2, Vec3( 7.0f, 8.0f, 9.0f ) );

	const Matrix33 m33Left  = Matrix33::V_Identity * m33;
	const Matrix33 m33Right = m33 * Matrix33::V_Identity;
	EXPECT_FLOAT_EQ( m33Left.GetRow( 1 ).y, m33.GetRow( 1 ).y );
	EXPECT_FLOAT_EQ( m33Right.GetRow( 2 ).x, m33.GetRow( 2 ).x );

	Matrix44 m44;
	m44.SetRow( 0, Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	m44.SetRow( 1, Vec4( 5.0f, 6.0f, 7.0f, 8.0f ) );
	m44.SetRow( 2, Vec4( 9.0f, 10.0f, 11.0f, 12.0f ) );
	m44.SetRow( 3, Vec4( 13.0f, 14.0f, 15.0f, 16.0f ) );

	const Matrix44 m44Left  = Matrix44::V_Identity * m44;
	const Matrix44 m44Right = m44 * Matrix44::V_Identity;
	EXPECT_FLOAT_EQ( m44Left.GetRow( 0 ).z, m44.GetRow( 0 ).z );
	EXPECT_FLOAT_EQ( m44Right.GetRow( 3 ).w, m44.GetRow( 3 ).w );
}

TEST( MatrixTests, Matrix44MultiplyProducesExpectedAffineProduct )
{
	Matrix44 translate;
	translate.SetRow( 0, Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	translate.SetRow( 1, Vec4( 0.0f, 1.0f, 0.0f, 2.0f ) );
	translate.SetRow( 2, Vec4( 0.0f, 0.0f, 1.0f, 3.0f ) );
	translate.SetRow( 3, Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	Matrix44 scale;
	scale.SetRow( 0, Vec4( 2.0f, 0.0f, 0.0f, 0.0f ) );
	scale.SetRow( 1, Vec4( 0.0f, 3.0f, 0.0f, 0.0f ) );
	scale.SetRow( 2, Vec4( 0.0f, 0.0f, 4.0f, 0.0f ) );
	scale.SetRow( 3, Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	const Matrix44 model = translate * scale;

	EXPECT_FLOAT_EQ( model.GetRow( 0 ).x, 2.0f );
	EXPECT_FLOAT_EQ( model.GetRow( 1 ).y, 3.0f );
	EXPECT_FLOAT_EQ( model.GetRow( 2 ).z, 4.0f );
	EXPECT_FLOAT_EQ( model.GetRow( 0 ).w, 1.0f );
	EXPECT_FLOAT_EQ( model.GetRow( 1 ).w, 2.0f );
	EXPECT_FLOAT_EQ( model.GetRow( 2 ).w, 3.0f );
	EXPECT_FLOAT_EQ( model.GetRow( 3 ).w, 1.0f );

	Matrix44 inplace = translate;
	inplace *= scale;
	EXPECT_FLOAT_EQ( inplace.GetRow( 0 ).x, model.GetRow( 0 ).x );
	EXPECT_FLOAT_EQ( inplace.GetRow( 1 ).y, model.GetRow( 1 ).y );
	EXPECT_FLOAT_EQ( inplace.GetRow( 2 ).z, model.GetRow( 2 ).z );
	EXPECT_FLOAT_EQ( inplace.GetRow( 0 ).w, model.GetRow( 0 ).w );
	EXPECT_FLOAT_EQ( inplace.GetRow( 1 ).w, model.GetRow( 1 ).w );
	EXPECT_FLOAT_EQ( inplace.GetRow( 2 ).w, model.GetRow( 2 ).w );
	EXPECT_FLOAT_EQ( inplace.GetRow( 3 ).w, model.GetRow( 3 ).w );
}

TEST( MatrixTests, Matrix33InverseProducesIdentityWhenMultiplied )
{
	Matrix33 m;
	m.SetRow( 0, Vec3( 4.0f, 7.0f, 2.0f ) );
	m.SetRow( 1, Vec3( 3.0f, 6.0f, 1.0f ) );
	m.SetRow( 2, Vec3( 2.0f, 5.0f, 3.0f ) );

	const Matrix33 inverse = m.Inverse();
	const Matrix33 left    = m * inverse;
	const Matrix33 right   = inverse * m;

	EXPECT_NEAR( left.GetRow( 0 ).x, 1.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 0 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 0 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).y, 1.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).z, 1.0f, 1e-5f );

	EXPECT_NEAR( right.GetRow( 0 ).x, 1.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 0 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 0 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).y, 1.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).z, 1.0f, 1e-5f );
}

TEST( MatrixTests, Matrix44InverseProducesIdentityWhenMultiplied )
{
	Matrix44 m;
	m.SetRow( 0, Vec4( 2.0f, 0.0f, 0.0f, 5.0f ) );
	m.SetRow( 1, Vec4( 0.0f, 3.0f, 0.0f, -4.0f ) );
	m.SetRow( 2, Vec4( 0.0f, 0.0f, 4.0f, 1.0f ) );
	m.SetRow( 3, Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	const Matrix44 inverse = m.Inverse();
	const Matrix44 left    = m * inverse;
	const Matrix44 right   = inverse * m;

	EXPECT_NEAR( left.GetRow( 0 ).x, 1.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 0 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 0 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 0 ).w, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).y, 1.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 1 ).w, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).z, 1.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 2 ).w, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 3 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 3 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 3 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( left.GetRow( 3 ).w, 1.0f, 1e-5f );

	EXPECT_NEAR( right.GetRow( 0 ).x, 1.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 0 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 0 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 0 ).w, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).y, 1.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 1 ).w, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).z, 1.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 2 ).w, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 3 ).x, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 3 ).y, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 3 ).z, 0.0f, 1e-5f );
	EXPECT_NEAR( right.GetRow( 3 ).w, 1.0f, 1e-5f );
}

TEST( MatrixTransformsTests, LookAtBuildsExpectedBasisAtOrigin )
{
	const Matrix44 view = LookAt44( Vec3::V_Zero, Vec3::V_Front, Vec3::V_Up );

	EXPECT_FLOAT_EQ( view.m00, 1.0f );
	EXPECT_FLOAT_EQ( view.m01, 0.0f );
	EXPECT_FLOAT_EQ( view.m02, 0.0f );
	EXPECT_FLOAT_EQ( view.m03, 0.0f );

	EXPECT_FLOAT_EQ( view.m10, 0.0f );
	EXPECT_FLOAT_EQ( view.m11, 1.0f );
	EXPECT_FLOAT_EQ( view.m12, 0.0f );
	EXPECT_FLOAT_EQ( view.m13, 0.0f );

	EXPECT_FLOAT_EQ( view.m20, 0.0f );
	EXPECT_FLOAT_EQ( view.m21, 0.0f );
	EXPECT_FLOAT_EQ( view.m22, 1.0f );
	EXPECT_FLOAT_EQ( view.m23, 0.0f );

	EXPECT_FLOAT_EQ( view.m30, 0.0f );
	EXPECT_FLOAT_EQ( view.m31, 0.0f );
	EXPECT_FLOAT_EQ( view.m32, 0.0f );
	EXPECT_FLOAT_EQ( view.m33, 1.0f );
}

TEST( MatrixTransformsTests, LookAt33BuildsExpectedBasis )
{
	const Matrix33 view3 = LookAt33( Vec3::V_Front, Vec3::V_Up );

	EXPECT_FLOAT_EQ( view3.m00, 1.0f );
	EXPECT_FLOAT_EQ( view3.m01, 0.0f );
	EXPECT_FLOAT_EQ( view3.m02, 0.0f );

	EXPECT_FLOAT_EQ( view3.m10, 0.0f );
	EXPECT_FLOAT_EQ( view3.m11, 1.0f );
	EXPECT_FLOAT_EQ( view3.m12, 0.0f );

	EXPECT_FLOAT_EQ( view3.m20, 0.0f );
	EXPECT_FLOAT_EQ( view3.m21, 0.0f );
	EXPECT_FLOAT_EQ( view3.m22, 1.0f );
}

TEST( MatrixTransformsTests, LookAt33HandlesParallelForwardAndUp )
{
	const Matrix33 view3 = LookAt33( Vec3::V_Up, Vec3::V_Up );

	const Vec3 row0( view3.m00, view3.m01, view3.m02 );
	const Vec3 row1( view3.m10, view3.m11, view3.m12 );
	const Vec3 row2( view3.m20, view3.m21, view3.m22 );

	EXPECT_NEAR( row0.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( row1.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( row2.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( Dot( row0, row1 ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( row0, row2 ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( row1, row2 ), 0.0f, 1e-5f );
}

TEST( MatrixTransformsTests, LookAt33HandlesZeroInputs )
{
	const Matrix33 view3 = LookAt33( Vec3::V_Zero, Vec3::V_Zero );

	const Vec3 row0 = view3.GetRow( 0 );
	const Vec3 row1 = view3.GetRow( 1 );
	const Vec3 row2 = view3.GetRow( 2 );

	EXPECT_NEAR( row0.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( row1.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( row2.LengthSq(), 1.0f, 1e-5f );
	EXPECT_NEAR( Dot( row0, row1 ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( row0, row2 ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( row1, row2 ), 0.0f, 1e-5f );
}

TEST( MatrixTransformsTests, LookAtComputesTranslationFromEye )
{
	const Vec3     eye( 2.0f, 3.0f, 4.0f );
	const Matrix44 view = LookAt44( eye, Vec3::V_Front, Vec3::V_Up );

	EXPECT_FLOAT_EQ( view.m03, -2.0f );
	EXPECT_FLOAT_EQ( view.m13, -3.0f );
	EXPECT_FLOAT_EQ( view.m23, -4.0f );
	EXPECT_FLOAT_EQ( view.m33, 1.0f );
}

TEST( MatrixTransformsTests, LookAt44MatchesLookAt33BasisAndComputedTranslation )
{
	const Vec3 eye( 2.0f, -1.0f, 5.0f );
	const Vec3 forward( 0.3f, 0.4f, 0.5f );
	const Vec3 up( 0.0f, 1.0f, 0.2f );

	const Matrix33 basis33 = LookAt33( forward, up );
	const Matrix44 view44  = LookAt44( eye, forward, up );

	EXPECT_NEAR( view44.m00, basis33.m00, 1e-5f );
	EXPECT_NEAR( view44.m10, basis33.m10, 1e-5f );
	EXPECT_NEAR( view44.m20, basis33.m20, 1e-5f );
	EXPECT_NEAR( view44.m01, basis33.m01, 1e-5f );
	EXPECT_NEAR( view44.m11, basis33.m11, 1e-5f );
	EXPECT_NEAR( view44.m21, basis33.m21, 1e-5f );
	EXPECT_NEAR( view44.m02, basis33.m02, 1e-5f );
	EXPECT_NEAR( view44.m12, basis33.m12, 1e-5f );
	EXPECT_NEAR( view44.m22, basis33.m22, 1e-5f );

	const float tx = -Dot( basis33.GetRow( 0 ), eye );
	const float ty = -Dot( basis33.GetRow( 1 ), eye );
	const float tz = Dot( -basis33.GetRow( 2 ), eye );

	EXPECT_NEAR( view44.m03, tx, 1e-5f );
	EXPECT_NEAR( view44.m13, ty, 1e-5f );
	EXPECT_NEAR( view44.m23, tz, 1e-5f );
	EXPECT_NEAR( view44.m33, 1.0f, 1e-5f );
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

	EXPECT_NEAR( p.m00, expectedM00, 1e-5f );
	EXPECT_NEAR( p.m11, expectedM11, 1e-5f );
	EXPECT_NEAR( p.m22, expectedM22, 1e-5f );
	EXPECT_NEAR( p.m32, -1.0f, 1e-5f );
	EXPECT_NEAR( p.m23, expectedM32, 1e-5f );
	EXPECT_NEAR( p.m33, 0.0f, 1e-5f );
}

TEST( MatrixTransformsTests, PerspectiveClampsInvalidInputs )
{
	const float    fov = ToRadians( 70.0f );
	const Matrix44 p   = Perspective( fov, 0.0f, -1.0f, -2.0f );

	const float clampedAspect = 1.0f;
	const float clampedNear   = 0.01f;
	const float clampedFar    = 0.02f;
	const float tanHalfFovY   = tanf( fov * 0.5f );

	EXPECT_NEAR( p.m00, 1.0f / ( tanHalfFovY * clampedAspect ), 1e-5f );
	EXPECT_NEAR( p.m11, 1.0f / tanHalfFovY, 1e-5f );
	EXPECT_NEAR( p.m22, ( clampedFar + clampedNear ) / ( clampedNear - clampedFar ), 1e-5f );
	EXPECT_NEAR( p.m23, ( 2.0f * clampedFar * clampedNear ) / ( clampedNear - clampedFar ), 1e-5f );
}
