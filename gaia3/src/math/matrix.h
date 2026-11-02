#pragma once

#include <math/vector.h>

struct Matrix33
{
	// clang-format off
	Matrix33()
	    : col_0( 0.0f, 0.0f, 0.0f )
		, col_1( 0.0f, 0.0f, 0.0f )
		, col_2( 0.0f, 0.0f, 0.0f )
	{
	}
	// clang-format on
	Matrix33( Vec3 _col_0, Vec3 _col_1, Vec3 _col_2 )
	    : col_0( _col_0 ), col_1( _col_1 ), col_2( _col_2 ) {}

	Matrix33  operator*( const Matrix33& rhs ) const;
	Matrix33& operator*=( const Matrix33& rhs );

	static const Matrix33 V_Identity;

	union
	{
		struct
		{
			Vec3 col_0;
			Vec3 col_1;
			Vec3 col_2;
		};

		Vec3 value[3];
	};

	void SetRow( int row, const Vec3& v );
	Vec3 GetRow( int row ) const;
};

struct Matrix44
{
	// clang-format off
	Matrix44()
	    : col_0( 0.0f, 0.0f, 0.0f, 0.0f )
		, col_1( 0.0f, 0.0f, 0.0f, 0.0f )
		, col_2( 0.0f, 0.0f, 0.0f, 0.0f )
		, col_3( 0.0f, 0.0f, 0.0f, 0.0f )
	{
	}
	explicit Matrix44( const Matrix33& m33 )
	    : col_0(    m33.col_0,     0.0f )
	    , col_1(    m33.col_1,     0.0f )
	    , col_2(    m33.col_2,     0.0f )
		, col_3( 0.0f, 0.0f, 0.0f, 0.0f )
	{
	}
	// clang-format on
	Matrix44( Vec4 _col_0, Vec4 _col_1, Vec4 _col_2, Vec4 _col_3 )
	    : col_0( _col_0 ), col_1( _col_1 ), col_2( _col_2 ), col_3( _col_3 ) {}

	Matrix44  operator*( const Matrix44& rhs ) const;
	Matrix44& operator*=( const Matrix44& rhs );

	static const Matrix44 V_Identity;

	union
	{
		struct
		{
			Vec4 col_0;
			Vec4 col_1;
			Vec4 col_2;
			Vec4 col_3;
		};

		Vec4 value[4];
	};

	void SetRow( int row, const Vec4& v );
	Vec4 GetRow( int row ) const;
};
