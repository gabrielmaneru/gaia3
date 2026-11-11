#pragma once

#include <math/vector.h>
#include <math/scalar.h>

struct Matrix33
{
	// clang-format off
	Matrix33()
		: m00(1.0f), m01(0.0f), m02(0.0f)
		, m10(0.0f), m11(1.0f), m12(0.0f)
		, m20(0.0f), m21(0.0f), m22(1.0f)
	{
	}
	// clang-format on
	explicit Matrix33( Vec3 _col0, Vec3 _col1, Vec3 _col2 )
	{
		col[0] = _col0;
		col[1] = _col1;
		col[2] = _col2;
	}

	Matrix33  operator*( const Matrix33& rhs ) const;
	Matrix33& operator*=( const Matrix33& rhs );
	Matrix33  operator*( const Scalar& s ) const;
	Matrix33& operator*=( const Scalar& s );
	Vec3      operator*( const Vec3& v ) const;

	static const Matrix33 V_Identity;

	union
	{
		struct
		{
			float m00;
			float m10;
			float m20;

			float m01;
			float m11;
			float m21;

			float m02;
			float m12;
			float m22;
		};
		Vec3 col[3];
	};

	void SetRow( int row, const Vec3& v );
	Vec3 GetRow( int row ) const;

	float    Determinant() const;
	Matrix33 Inverse() const;
};

struct Matrix44
{
	// clang-format off
	Matrix44()
		: m00(1.0f), m01(0.0f), m02(0.0f), m03(0.0f)
		, m10(0.0f), m11(1.0f), m12(0.0f), m13(0.0f)
		, m20(0.0f), m21(0.0f), m22(1.0f), m23(0.0f)
		, m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
	{
	}
	explicit Matrix44( const Matrix33& m33 )
		: m00(m33.m00), m01(m33.m01), m02(m33.m02), m03(0.0f)
		, m10(m33.m10), m11(m33.m11), m12(m33.m12), m13(0.0f)
		, m20(m33.m20), m21(m33.m21), m22(m33.m22), m23(0.0f)
		, m30(0.0f),    m31(0.0f),    m32(0.0f),    m33(1.0f)
	{
	}
	// clang-format on
	explicit Matrix44( Vec4 _col0, Vec4 _col1, Vec4 _col2, Vec4 _col3 )
	{
		col[0] = _col0;
		col[1] = _col1;
		col[2] = _col2;
		col[3] = _col3;
	}

	Matrix44  operator*( const Matrix44& rhs ) const;
	Matrix44& operator*=( const Matrix44& rhs );
	Matrix44  operator*( const Scalar& s ) const;
	Matrix44& operator*=( const Scalar& s );
	Vec4      operator*( const Vec4& v ) const;

	static const Matrix44 V_Identity;

	union
	{
		struct
		{
			float m00;
			float m10;
			float m20;
			float m30;

			float m01;
			float m11;
			float m21;
			float m31;

			float m02;
			float m12;
			float m22;
			float m32;

			float m03;
			float m13;
			float m23;
			float m33;
		};
		Vec4 col[4];
	};

	void SetRow( int row, const Vec4& v );
	Vec4 GetRow( int row ) const;

	float    Determinant() const;
	Matrix44 Inverse() const;
};
