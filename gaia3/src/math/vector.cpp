#include "gaia3_pch.h"
#include "vector.h"

/// Vec3
const Vec3 Vec3::V_Zero( 0.0f, 0.0f, 0.0f );
const Vec3 Vec3::V_Front( 1.0f, 0.0f, 0.0f );
const Vec3 Vec3::V_Up( 0.0f, 1.0f, 0.0f );
const Vec3 Vec3::V_Right( 0.0f, 0.0f, 1.0f );

Vec3 Vec3::operator-() const
{
	return Vec3( -x, -y, -z );
}

bool Vec3::operator==( const Vec3& rhs ) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool Vec3::operator!=( const Vec3& rhs ) const
{
	return !( *this == rhs );
}

Vec3 Vec3::operator+( const Vec3& rhs ) const
{
	Vec3 result( *this );
	return result += rhs;
}

Vec3& Vec3::operator+=( const Vec3& rhs )
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

Vec3 Vec3::operator-( const Vec3& rhs ) const
{
	Vec3 result( *this );
	return result -= rhs;
}

Vec3& Vec3::operator-=( const Vec3& rhs )
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

Vec3 Vec3::operator*( const Scalar& s ) const
{
	Vec3 result( *this );
	return result *= s;
}

Vec3& Vec3::operator*=( const Scalar& s )
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

float Vec3::LengthSq() const
{
	return ( x * x ) + ( y * y ) + ( z * z );
}

void Vec3::Normalize()
{
	const float lengthSq = LengthSq();
	if( lengthSq <= 0.0f )
	{
		*this = Vec3::V_Zero;
	}
	else
	{
		const Scalar invLength( 1.0f / sqrtf( lengthSq ) );
		*this *= invLength;
	}
}

Vec3 Vec3::Normalized() const
{
	Vec3 v = *this;
	v.Normalize();
	return v;
}

/// Vec4
const Vec4 Vec4::V_Zero( 0.0f, 0.0f, 0.0f );
const Vec4 Vec4::V_Front( 1.0f, 0.0f, 0.0f );
const Vec4 Vec4::V_Up( 0.0f, 1.0f, 0.0f );
const Vec4 Vec4::V_Right( 0.0f, 0.0f, 1.0f );

Vec4 Vec4::operator-() const
{
	return Vec4( -x, -y, -z, -w );
}

bool Vec4::operator==( const Vec4& rhs ) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

bool Vec4::operator!=( const Vec4& rhs ) const
{
	return !( *this == rhs );
}

Vec4 Vec4::operator+( const Vec4& rhs ) const
{
	Vec4 result( *this );
	return result += rhs;
}

Vec4& Vec4::operator+=( const Vec4& rhs )
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

Vec4 Vec4::operator-( const Vec4& rhs ) const
{
	Vec4 result( *this );
	return result -= rhs;
}

Vec4& Vec4::operator-=( const Vec4& rhs )
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

Vec4 Vec4::operator*( const Scalar& s ) const
{
	Vec4 result( *this );
	return result *= s;
}

Vec4& Vec4::operator*=( const Scalar& s )
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

float Vec4::LengthSq() const
{
	return ( x * x ) + ( y * y ) + ( z * z ) + ( w * w );
}

void Vec4::Normalize()
{
	const float lengthSq = LengthSq();
	if( lengthSq <= 0.0f )
	{
		*this = Vec4::V_Zero;
	}
	else
	{
		const Scalar invLength( 1.0f / sqrtf( lengthSq ) );
		*this *= invLength;
	}
}

Vec4 Vec4::Normalized() const
{
	Vec4 v = *this;
	v.Normalize();
	return v;
}
