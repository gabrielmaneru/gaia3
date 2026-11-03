#pragma once

#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/vector.h>

struct Transform
{
	Transform() = default;

	explicit Transform( Vec3 _pos, Quat _rot, Vec3 _scl )
	    : position( _pos ), rotation( _rot ), scale( _scl ) {}

	Transform  operator*( const Transform& rhs ) const;
	Transform& operator*=( const Transform& rhs );

	static const Transform V_Identity;

	Vec3 position = Vec3();
	Quat rotation = Quat();
	Vec3 scale    = Vec3( 1.0f, 1.0f, 1.0f );

	Transform Inverse() const;
	Matrix44  ToMatrix44() const;
};