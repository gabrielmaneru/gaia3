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
	// clang-format on

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
};
