#pragma once

#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/vector.h>

struct Transform
{
	Transform( Vec3 _pos = Vec3::V_Zero, Quat _rot = Quat::V_Identity, Vec3 _scl = Vec3::V_One )
	    : position( _pos ), rotation( _rot ), scale( _scl ) {}

	Transform  operator*( const Transform& rhs ) const;
	Transform& operator*=( const Transform& rhs );

	static const Transform V_Identity;

	Vec3 position;
	Quat rotation;
	Vec3 scale;

	Matrix44 ToMatrix44() const;
};