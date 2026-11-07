#pragma once

#include <math/scalar.h>
#include <math/vector.h>

struct Quat
{
	Quat() = default;
	explicit Quat( float _x, float _y, float _z, float _w )
	    : x( _x ), y( _y ), z( _z ), w( _w )
	{
	}

	Quat  operator-() const;
	Quat  operator*( const Quat& rhs ) const;
	Quat& operator*=( const Quat& rhs );
	Quat  operator*( const Scalar& s ) const;
	Quat& operator*=( const Scalar& s );

	static const Quat V_Identity;
	static const Quat V_Zero;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	float LengthSq() const;
	void  Normalize();
	Quat  Normalized() const;
	Quat  Inverse() const;

	Vec3 Rotate( const Vec3& v ) const;
	Vec4 Rotate( const Vec4& v ) const;
};