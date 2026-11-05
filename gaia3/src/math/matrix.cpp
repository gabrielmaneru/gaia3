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

	m00 = Dot( row0, rhs.col[0] );
	m01 = Dot( row0, rhs.col[1] );
	m02 = Dot( row0, rhs.col[2] );

	m10 = Dot( row1, rhs.col[0] );
	m11 = Dot( row1, rhs.col[1] );
	m12 = Dot( row1, rhs.col[2] );

	m20 = Dot( row2, rhs.col[0] );
	m21 = Dot( row2, rhs.col[1] );
	m22 = Dot( row2, rhs.col[2] );

	return *this;
}

Matrix33 Matrix33::operator*( const Scalar& s ) const
{
	Matrix33 result( *this );
	return result *= s;
}

Matrix33& Matrix33::operator*=( const Scalar& s )
{
	col[0] *= s;
	col[1] *= s;
	col[2] *= s;
	return *this;
}

void Matrix33::SetRow( int row, const Vec3& v )
{
	switch( row )
	{
	case 0:
		m00 = v.x;
		m01 = v.y;
		m02 = v.z;
		break;
	case 1:
		m10 = v.x;
		m11 = v.y;
		m12 = v.z;
		break;
	case 2:
		m20 = v.x;
		m21 = v.y;
		m22 = v.z;
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
		return Vec3( m00, m01, m02 );
	case 1:
		return Vec3( m10, m11, m12 );
	case 2:
		return Vec3( m20, m21, m22 );
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

	m00 = Dot( row0, rhs.col[0] );
	m01 = Dot( row0, rhs.col[1] );
	m02 = Dot( row0, rhs.col[2] );
	m03 = Dot( row0, rhs.col[3] );

	m10 = Dot( row1, rhs.col[0] );
	m11 = Dot( row1, rhs.col[1] );
	m12 = Dot( row1, rhs.col[2] );
	m13 = Dot( row1, rhs.col[3] );

	m20 = Dot( row2, rhs.col[0] );
	m21 = Dot( row2, rhs.col[1] );
	m22 = Dot( row2, rhs.col[2] );
	m23 = Dot( row2, rhs.col[3] );

	m30 = Dot( row3, rhs.col[0] );
	m31 = Dot( row3, rhs.col[1] );
	m32 = Dot( row3, rhs.col[2] );
	m33 = Dot( row3, rhs.col[3] );

	return *this;
}

Matrix44 Matrix44::operator*( const Scalar& s ) const
{
	Matrix44 result( *this );
	return result *= s;
}

Matrix44& Matrix44::operator*=( const Scalar& s )
{
	col[0] *= s;
	col[1] *= s;
	col[2] *= s;
	col[3] *= s;
	return *this;
}

void Matrix44::SetRow( int row, const Vec4& v )
{
	switch( row )
	{
	case 0:
		m00 = v.x;
		m01 = v.y;
		m02 = v.z;
		m03 = v.w;
		break;
	case 1:
		m10 = v.x;
		m11 = v.y;
		m12 = v.z;
		m13 = v.w;
		break;
	case 2:
		m20 = v.x;
		m21 = v.y;
		m22 = v.z;
		m23 = v.w;
		break;
	case 3:
		m30 = v.x;
		m31 = v.y;
		m32 = v.z;
		m33 = v.w;
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
		return Vec4( m00, m01, m02, m03 );
	case 1:
		return Vec4( m10, m11, m12, m13 );
	case 2:
		return Vec4( m20, m21, m22, m23 );
	case 3:
		return Vec4( m30, m31, m32, m33 );
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

	return Dot( col[0], Vec4( inv00, -inv01, inv02, -inv03 ) );
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

	const float det = Dot( col[0], inverse.GetRow( 0 ) );

	const Scalar invDet( 1.0f / det );

	return inverse * invDet;
}
