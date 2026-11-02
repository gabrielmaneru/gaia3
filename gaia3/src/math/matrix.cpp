#include "gaia3_pch.h"
#include "matrix.h"

const Matrix33 Matrix33::V_Identity(
    Vec3( 1.0f, 0.0f, 0.0f ),
    Vec3( 0.0f, 1.0f, 0.0f ),
    Vec3( 0.0f, 0.0f, 1.0f ) );

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

const Matrix44 Matrix44::V_Identity(
    Vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
    Vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
    Vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
    Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

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
