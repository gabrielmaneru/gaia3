#include "mathHelpers.h"

#include <core/log.h>

#define _USE_MATH_DEFINES
#include <math.h>

float ToRadians( float degrees )
{
	return degrees * ( static_cast<float>( M_PI ) / 180.0f );
}

float ToDegrees( float radians )
{
	return radians * ( 180.0f / static_cast<float>( M_PI ) );
}

float Clamp( float value, float minValue, float maxValue )
{
	if( value < minValue )
		return minValue;
	if( value > maxValue )
		return maxValue;
	return value;
}

float Ramp( float value, float minValue, float maxValue )
{
	if( maxValue <= minValue )
	{
		GAIA_WARN( "Ramp: empty range {} to {}, using zero", minValue, maxValue );
		return 0.0f;
	}

	return Clamp( ( value - minValue ) / ( maxValue - minValue ), 0.0f, 1.0f );
}

Vec2 Min( const Vec2& a, const Vec2& b )
{
	return Vec2( Min( a.x, b.x ), Min( a.y, b.y ) );
}

Vec3 Min( const Vec3& a, const Vec3& b )
{
	return Vec3( Min( a.x, b.x ), Min( a.y, b.y ), Min( a.z, b.z ) );
}

Vec4 Min( const Vec4& a, const Vec4& b )
{
	return Vec4( Min( a.x, b.x ), Min( a.y, b.y ), Min( a.z, b.z ), Min( a.w, b.w ) );
}

Vec2 Max( const Vec2& a, const Vec2& b )
{
	return Vec2( Max( a.x, b.x ), Max( a.y, b.y ) );
}

Vec3 Max( const Vec3& a, const Vec3& b )
{
	return Vec3( Max( a.x, b.x ), Max( a.y, b.y ), Max( a.z, b.z ) );
}

Vec4 Max( const Vec4& a, const Vec4& b )
{
	return Vec4( Max( a.x, b.x ), Max( a.y, b.y ), Max( a.z, b.z ), Max( a.w, b.w ) );
}

bool IsClose( const Vec2& a, const Vec2& b, float epsilon )
{
	return fabsf( a.x - b.x ) <= epsilon
	       && fabsf( a.y - b.y ) <= epsilon;
}

bool IsClose( const Vec3& a, const Vec3& b, float epsilon )
{
	return fabsf( a.x - b.x ) <= epsilon
	       && fabsf( a.y - b.y ) <= epsilon
	       && fabsf( a.z - b.z ) <= epsilon;
}

bool IsClose( const Vec4& a, const Vec4& b, float epsilon )
{
	return fabsf( a.x - b.x ) <= epsilon
	       && fabsf( a.y - b.y ) <= epsilon
	       && fabsf( a.z - b.z ) <= epsilon
	       && fabsf( a.w - b.w ) <= epsilon;
}
