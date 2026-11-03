#include "gaia3_pch.h"
#include "matrix.h"

#include <math.h>

const Matrix33 Matrix33::V_Identity = Matrix33();

Matrix33 Matrix33::operator*( const Matrix33& rhs ) const
{
	Matrix33 result( *this );
	return result *= rhs;
}

Matrix33& Matrix33::operator*=( const Matrix33& rhs )
{
	const Matrix33 lhs( *this );

	const Vec3 row0 = lhs.GetRow( 0 );
	const Vec3 row1 = lhs.GetRow( 1 );
	const Vec3 row2 = lhs.GetRow( 2 );

	col_0.x = Dot( row0, rhs.col_0 );
	col_0.y = Dot( row1, rhs.col_0 );
	col_0.z = Dot( row2, rhs.col_0 );

	col_1.x = Dot( row0, rhs.col_1 );
	col_1.y = Dot( row1, rhs.col_1 );
	col_1.z = Dot( row2, rhs.col_1 );

	col_2.x = Dot( row0, rhs.col_2 );
	col_2.y = Dot( row1, rhs.col_2 );
	col_2.z = Dot( row2, rhs.col_2 );

	return *this;
}

Matrix33 Matrix33::operator*( const Scalar& s ) const
{
	Matrix33 result( *this );
	return result *= s;
}

Matrix33& Matrix33::operator*=( const Scalar& s )
{
	col_0 *= s;
	col_1 *= s;
	col_2 *= s;
	return *this;
}

void Matrix33::SetRow( int row, const Vec3& v )
{
	switch( row )
	{
	case 0:
		col_0.x = v.x;
		col_1.x = v.y;
		col_2.x = v.z;
		break;
	case 1:
		col_0.y = v.x;
		col_1.y = v.y;
		col_2.y = v.z;
		break;
	case 2:
		col_0.z = v.x;
		col_1.z = v.y;
		col_2.z = v.z;
		break;
	default:
		assert( false );
		break;
	}
}

Vec3 Matrix33::GetRow( int row ) const
{
	switch( row )
	{
	case 0:
		return Vec3( col_0.x, col_1.x, col_2.x );
	case 1:
		return Vec3( col_0.y, col_1.y, col_2.y );
	case 2:
		return Vec3( col_0.z, col_1.z, col_2.z );
	default:
		assert( false );
		break;
	}

	return Vec3::V_Zero;
}

float Matrix33::Determinant() const
{
	const float c00 = m11 * m22 - m12 * m21;
	const float c01 = m12 * m20 - m10 * m22;
	const float c02 = m10 * m21 - m11 * m20;
	return m00 * c00 + m01 * c01 + m02 * c02;
}

Matrix33 Matrix33::Inverse() const
{
	const float c00 = m11 * m22 - m12 * m21;
	const float c01 = m12 * m20 - m10 * m22;
	const float c02 = m10 * m21 - m11 * m20;

	const float c10 = m02 * m21 - m01 * m22;
	const float c11 = m00 * m22 - m02 * m20;
	const float c12 = m01 * m20 - m00 * m21;

	const float c20 = m01 * m12 - m02 * m11;
	const float c21 = m02 * m10 - m00 * m12;
	const float c22 = m00 * m11 - m01 * m10;

	const Vec3 Inv0( c00, c01, c02 );
	const Vec3 Inv1( c10, c11, c12 );
	const Vec3 Inv2( c20, c21, c22 );

	const float det = m00 * c00 + m01 * c01 + m02 * c02;
	if( fabsf( det ) <= eps_6 )
	{
		assert( false );
		return Matrix33();
	}

	Matrix33 inverse( Inv0, Inv1, Inv2 );

	const Scalar invDet( 1.0f / det );

	return inverse * invDet;
}

const Matrix44 Matrix44::V_Identity = Matrix44();

Matrix44 Matrix44::operator*( const Matrix44& rhs ) const
{
	Matrix44 result( *this );
	return result *= rhs;
}

Matrix44& Matrix44::operator*=( const Matrix44& rhs )
{
	const Matrix44 lhs( *this );

	const Vec4 row0 = lhs.GetRow( 0 );
	const Vec4 row1 = lhs.GetRow( 1 );
	const Vec4 row2 = lhs.GetRow( 2 );
	const Vec4 row3 = lhs.GetRow( 3 );

	col_0.x = Dot( row0, rhs.col_0 );
	col_0.y = Dot( row1, rhs.col_0 );
	col_0.z = Dot( row2, rhs.col_0 );
	col_0.w = Dot( row3, rhs.col_0 );

	col_1.x = Dot( row0, rhs.col_1 );
	col_1.y = Dot( row1, rhs.col_1 );
	col_1.z = Dot( row2, rhs.col_1 );
	col_1.w = Dot( row3, rhs.col_1 );

	col_2.x = Dot( row0, rhs.col_2 );
	col_2.y = Dot( row1, rhs.col_2 );
	col_2.z = Dot( row2, rhs.col_2 );
	col_2.w = Dot( row3, rhs.col_2 );

	col_3.x = Dot( row0, rhs.col_3 );
	col_3.y = Dot( row1, rhs.col_3 );
	col_3.z = Dot( row2, rhs.col_3 );
	col_3.w = Dot( row3, rhs.col_3 );

	return *this;
}

Matrix44 Matrix44::operator*( const Scalar& s ) const
{
	Matrix44 result( *this );
	return result *= s;
}

Matrix44& Matrix44::operator*=( const Scalar& s )
{
	col_0 *= s;
	col_1 *= s;
	col_2 *= s;
	col_3 *= s;
	return *this;
}

void Matrix44::SetRow( int row, const Vec4& v )
{
	switch( row )
	{
	case 0:
		col_0.x = v.x;
		col_1.x = v.y;
		col_2.x = v.z;
		col_3.x = v.w;
		break;
	case 1:
		col_0.y = v.x;
		col_1.y = v.y;
		col_2.y = v.z;
		col_3.y = v.w;
		break;
	case 2:
		col_0.z = v.x;
		col_1.z = v.y;
		col_2.z = v.z;
		col_3.z = v.w;
		break;
	case 3:
		col_0.w = v.x;
		col_1.w = v.y;
		col_2.w = v.z;
		col_3.w = v.w;
		break;
	default:
		assert( false );
		break;
	}
}

Vec4 Matrix44::GetRow( int row ) const
{
	switch( row )
	{
	case 0:
		return Vec4( col_0.x, col_1.x, col_2.x, col_3.x );
	case 1:
		return Vec4( col_0.y, col_1.y, col_2.y, col_3.y );
	case 2:
		return Vec4( col_0.z, col_1.z, col_2.z, col_3.z );
	case 3:
		return Vec4( col_0.w, col_1.w, col_2.w, col_3.w );
	default:
		assert( false );
		break;
	}

	return Vec4::V_Zero;
}

float Matrix44::Determinant() const
{
	const float c00 = m22 * m33 - m23 * m32;
	const float c04 = m12 * m33 - m13 * m32;
	const float c08 = m12 * m23 - m13 * m22;
	const float c12 = m02 * m33 - m03 * m32;
	const float c16 = m02 * m23 - m03 * m22;
	const float c20 = m02 * m13 - m03 * m12;

	const float inv00( m11 * c00 - m21 * c04 + m31 * c08 );
	const float inv01( m01 * c00 - m21 * c12 + m31 * c16 );
	const float inv02( m01 * c04 - m11 * c12 + m31 * c20 );
	const float inv03( m01 * c08 - m11 * c16 + m21 * c20 );

	return Dot( col_0, Vec4( inv00, -inv01, inv02, -inv03 ) );
}

Matrix44 Matrix44::Inverse() const
{
	const float c00 = m22 * m33 - m23 * m32;
	const float c02 = m21 * m33 - m23 * m31;
	const float c03 = m21 * m32 - m22 * m31;
	const float c04 = m12 * m33 - m13 * m32;
	const float c06 = m11 * m33 - m13 * m31;
	const float c07 = m11 * m32 - m12 * m31;
	const float c08 = m12 * m23 - m13 * m22;
	const float c10 = m11 * m23 - m13 * m21;
	const float c11 = m11 * m22 - m12 * m21;
	const float c12 = m02 * m33 - m03 * m32;
	const float c14 = m01 * m33 - m03 * m31;
	const float c15 = m01 * m32 - m02 * m31;
	const float c16 = m02 * m23 - m03 * m22;
	const float c18 = m01 * m23 - m03 * m21;
	const float c19 = m01 * m22 - m02 * m21;

	const float c20 = m02 * m13 - m03 * m12;
	const float c22 = m01 * m13 - m03 * m11;
	const float c23 = m01 * m12 - m02 * m11;

	const Vec4 fac0( c00, c00, c02, c03 );
	const Vec4 fac1( c04, c04, c06, c07 );
	const Vec4 fac2( c08, c08, c10, c11 );
	const Vec4 fac3( c12, c12, c14, c15 );
	const Vec4 fac4( c16, c16, c18, c19 );
	const Vec4 fac5( c20, c20, c22, c23 );

	const Vec4 vec0( m01, m00, m00, m00 );
	const Vec4 vec1( m11, m10, m10, m10 );
	const Vec4 vec2( m21, m20, m20, m20 );
	const Vec4 vec3( m31, m30, m30, m30 );

	const Vec4 inv0( vec1 * fac0 - vec2 * fac1 + vec3 * fac2 );
	const Vec4 inv1( vec0 * fac0 - vec2 * fac3 + vec3 * fac4 );
	const Vec4 inv2( vec0 * fac1 - vec1 * fac3 + vec3 * fac5 );
	const Vec4 inv3( vec0 * fac2 - vec1 * fac4 + vec2 * fac5 );

	const Vec4 signA( +1, -1, +1, -1 );
	const Vec4 signB( -1, +1, -1, +1 );

	Matrix44 inverse( inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB );

	const float det = Dot( col_0, inverse.GetRow( 0 ) );

	const Scalar invDet( 1.0f / det );

	return inverse * invDet;
}
