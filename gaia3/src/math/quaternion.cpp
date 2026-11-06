#include "gaia3_pch.h"
#include "quaternion.h"

#include <math.h>

const Quat Quat::V_Identity = Quat();
const Quat Quat::V_Zero( 0.0f, 0.0f, 0.0f, 0.0f );

Quat Quat::operator-() const
{
	return Quat( -x, -y, -z, -w );
}

Quat Quat::operator*( const Quat& rhs ) const
{
	Quat result( *this );
	return result *= rhs;
}

Quat& Quat::operator*=( const Quat& rhs )
{
	const Quat lhs( *this );

	// Note: Hamilton product, so ( a * b ).Rotate( v ) applies b first, matching matrix multiplication
	x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
	y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z;
	z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x;
	w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
	return *this;
}

Quat Quat::operator*( const Scalar& s ) const
{
	Quat result( *this );
	return result *= s;
}

Quat& Quat::operator*=( const Scalar& s )
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

float Quat::LengthSq() const
{
	return ( x * x ) + ( y * y ) + ( z * z ) + ( w * w );
}

void Quat::Normalize()
{
	const float lengthSq = LengthSq();
	if( lengthSq <= 0.0f )
	{
		*this = Quat::V_Identity;
	}
	else
	{
		const Scalar invLength( 1.0f / sqrtf( lengthSq ) );
		*this *= invLength;
	}
}

Quat Quat::Normalized() const
{
	Quat v = *this;
	v.Normalize();
	return v;
}

Quat Quat::Inverse() const
{
	return Quat( -x, -y, -z, w );
}

Vec3 Quat::Rotate( const Vec3& v ) const
{
	const Vec3 qv( x, y, z );
	const Vec3 t = Cross( qv, v ) * Scalar( 2.0f );
	return v + ( t * Scalar( w ) ) + Cross( qv, t );
}

Vec4 Quat::Rotate( const Vec4& v ) const
{
	Vec3 v3 = Vec3( v );
	return Vec4( Rotate( v3 ), v.w );
}
