#pragma once

#include <math/vector.h>

#define eps_6 1e-6f

float ToRadians( float degrees );
float ToDegrees( float radians );
float Clamp( float value, float minValue, float maxValue );
float Ramp( float value, float minValue, float maxValue );

bool IsClose( const Vec3& a, const Vec3& b, float epsilon );
bool IsClose( const Vec4& a, const Vec4& b, float epsilon );
