#include "gaia3_pch.h"
#include "mathHelpers.h"

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
		return 0.0f;

	return Clamp( ( value - minValue ) / ( maxValue - minValue ), 0.0f, 1.0f );
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
