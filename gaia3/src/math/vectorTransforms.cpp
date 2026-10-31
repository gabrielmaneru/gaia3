#include "gaia3_pch.h"
#include "vectorTransforms.h"

#include <math.h>

Vec3 Cross( const Vec3& a, const Vec3& b )
{
	return Vec3( a.y * b.z - a.z * b.y,
	             a.z * b.x - a.x * b.z,
	             a.x * b.y - a.y * b.x );
}

Vec4 Cross( const Vec4& a, const Vec4& b )
{
	return Vec4( a.y * b.z - a.z * b.y,
	             a.z * b.x - a.x * b.z,
	             a.x * b.y - a.y * b.x,
	             0.0f );
}

float Dot( const Vec3& a, const Vec3& b )
{
	return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
}

float Dot( const Vec4& a, const Vec4& b )
{
	return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) + ( a.w * b.w );
}
