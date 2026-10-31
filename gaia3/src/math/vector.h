#pragma once

#include <math/scalar.h>

struct Vec3
{
	Vec3( float _x = 0.0f, float _y = 0.0f, float _z = 0.0f )
	    : x( _x ), y( _y ), z( _z )
	{
	}

	Vec3  operator-() const;
	bool  operator==( const Vec3& rhs ) const;
	bool  operator!=( const Vec3& rhs ) const;
	Vec3  operator+( const Vec3& rhs ) const;
	Vec3& operator+=( const Vec3& rhs );
	Vec3  operator-( const Vec3& rhs ) const;
	Vec3& operator-=( const Vec3& rhs );
	Vec3  operator*( const Scalar& s ) const;
	Vec3& operator*=( const Scalar& s );

	static const Vec3 V_Zero;
	static const Vec3 V_Front;
	static const Vec3 V_Up;
	static const Vec3 V_Right;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	float LengthSq() const;
	void  Normalize();
	Vec3  Normalized() const;
};

struct Vec4
{
	Vec4( const Vec3& _v, float _w = 1.0f )
	    : x( _v.x ), y( _v.y ), z( _v.z ), w( _w )
	{
	}
	Vec4( float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 1.0f )
	    : x( _x ), y( _y ), z( _z ), w( _w )
	{
	}

	Vec4  operator-() const;
	bool  operator==( const Vec4& rhs ) const;
	bool  operator!=( const Vec4& rhs ) const;
	Vec4  operator+( const Vec4& rhs ) const;
	Vec4& operator+=( const Vec4& rhs );
	Vec4  operator-( const Vec4& rhs ) const;
	Vec4& operator-=( const Vec4& rhs );
	Vec4  operator*( const Scalar& s ) const;
	Vec4& operator*=( const Scalar& s );

	static const Vec4 V_Zero;
	static const Vec4 V_Front;
	static const Vec4 V_Up;
	static const Vec4 V_Right;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	float LengthSq() const;
	void  Normalize();
	Vec4  Normalized() const;
};
