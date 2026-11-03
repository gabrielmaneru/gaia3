#include "gaia3_pch.h"
#include "quaternion.h"

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

	if( false )
	{
		const Vec3 v1( lhs.x, lhs.y, lhs.z );
		const Vec3 v2( rhs.x, rhs.y, rhs.z );

		const Vec3 term1 = v2 * Scalar( lhs.w );
		const Vec3 term2 = v1 * Scalar( rhs.w );
		const Vec3 term3 = Cross( v1, v2 );

		x = term1.x + term2.x + term3.x;
		y = term1.y + term2.y + term3.y;
		z = term1.z + term2.z + term3.z;
		w = w * rhs.w - Dot( v1, v2 );
	}

	x = lhs.w * lhs.x + rhs.x * lhs.w + rhs.y * lhs.z - rhs.z * lhs.y;
	y = lhs.w * lhs.y + rhs.y * lhs.w + rhs.z * lhs.x - rhs.x * lhs.z;
	z = lhs.w * lhs.z + rhs.z * lhs.w + rhs.x * lhs.y - rhs.y * lhs.x;
	w = lhs.w * lhs.w - rhs.x * lhs.x - rhs.y * lhs.y - rhs.z * lhs.z;
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
