#pragma once

#include <math/vector.h>

#define eps_6 1e-6f

float ToRadians( float degrees );
float ToDegrees( float radians );
float Clamp( float value, float minValue, float maxValue );
float Ramp( float value, float minValue, float maxValue );

// Note: The smaller and larger of two values of any type with operator<
// Mixed types need the type spelled out, like Min<u64>( a, b )
template <typename T>
constexpr T Min( const T& a, const T& b )
{
	return b < a ? b : a;
}

template <typename T>
constexpr T Max( const T& a, const T& b )
{
	return a < b ? b : a;
}

// Note: Component by component, each component the smaller or larger of the two
Vec2 Min( const Vec2& a, const Vec2& b );
Vec3 Min( const Vec3& a, const Vec3& b );
Vec4 Min( const Vec4& a, const Vec4& b );
Vec2 Max( const Vec2& a, const Vec2& b );
Vec3 Max( const Vec3& a, const Vec3& b );
Vec4 Max( const Vec4& a, const Vec4& b );

bool IsClose( const Vec2& a, const Vec2& b, float epsilon );
bool IsClose( const Vec3& a, const Vec3& b, float epsilon );
bool IsClose( const Vec4& a, const Vec4& b, float epsilon );
