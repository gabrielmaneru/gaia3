#pragma once

#include <math/scalar.h>

struct Vec2;
struct Vec3;
struct Vec4;

struct Vec2
{
	Vec2() = default;
	explicit Vec2( float _x, float _y )
	    : x( _x ), y( _y )
	{
	}

	Vec2  operator-() const;
	bool  operator==( const Vec2& rhs ) const;
	bool  operator!=( const Vec2& rhs ) const;
	Vec2  operator+( const Vec2& rhs ) const;
	Vec2& operator+=( const Vec2& rhs );
	Vec2  operator-( const Vec2& rhs ) const;
	Vec2& operator-=( const Vec2& rhs );
	Vec2  operator*( const Vec2& rhs ) const;
	Vec2& operator*=( const Vec2& rhs );
	Vec2  operator*( const Scalar& s ) const;
	Vec2& operator*=( const Scalar& s );

	static const Vec2 V_Zero;
	static const Vec2 V_One;
	static const Vec2 V_Right;
	static const Vec2 V_Up;

	float x = 0.0f;
	float y = 0.0f;

	float LengthSq() const;
	void  Normalize();
	Vec2  Normalized() const;
};

struct Vec3
{
	Vec3() = default;
	explicit Vec3( float _x, float _y, float _z )
	    : x( _x ), y( _y ), z( _z )
	{
	}
	explicit Vec3( const Vec2& _v, float _z = 0.0f );
	explicit Vec3( const Vec4& _v );

	Vec3  operator-() const;
	bool  operator==( const Vec3& rhs ) const;
	bool  operator!=( const Vec3& rhs ) const;
	Vec3  operator+( const Vec3& rhs ) const;
	Vec3& operator+=( const Vec3& rhs );
	Vec3  operator-( const Vec3& rhs ) const;
	Vec3& operator-=( const Vec3& rhs );
	Vec3  operator*( const Vec3& rhs ) const;
	Vec3& operator*=( const Vec3& rhs );
	Vec3  operator*( const Scalar& s ) const;
	Vec3& operator*=( const Scalar& s );

	static const Vec3 V_Zero;
	static const Vec3 V_One;
	static const Vec3 V_Right;
	static const Vec3 V_Up;
	static const Vec3 V_Front;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	float LengthSq() const;
	void  Normalize();
	Vec3  Normalized() const;
};

struct Vec4
{
	Vec4() = default;
	explicit Vec4( float _x, float _y, float _z, float _w )
	    : x( _x ), y( _y ), z( _z ), w( _w )
	{
	}
	explicit Vec4( const Vec2& _v, float _z = 0.0f, float _w = 0.0f );
	explicit Vec4( const Vec3& _v, float _w = 0.0f );

	Vec4  operator-() const;
	bool  operator==( const Vec4& rhs ) const;
	bool  operator!=( const Vec4& rhs ) const;
	Vec4  operator+( const Vec4& rhs ) const;
	Vec4& operator+=( const Vec4& rhs );
	Vec4  operator-( const Vec4& rhs ) const;
	Vec4& operator-=( const Vec4& rhs );
	Vec4  operator*( const Vec4& rhs ) const;
	Vec4& operator*=( const Vec4& rhs );
	Vec4  operator*( const Scalar& s ) const;
	Vec4& operator*=( const Scalar& s );

	static const Vec4 V_Zero;
	static const Vec4 V_One;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	float LengthSq() const;
	void  Normalize();
	Vec4  Normalized() const;
};
