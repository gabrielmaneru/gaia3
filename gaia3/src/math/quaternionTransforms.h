#pragma once

#include <math/matrix.h>
#include <math/quaternion.h>

float Dot( const Quat& a, const Quat& b );

Quat QuatFromMatrix33( const Matrix33& m );

Quat LookAtQuat( const Vec3& forwardDir, const Vec3& upAxis );

// Note: Angles are degrees around x, y and z, applied in the order z, then x, then y
Quat QuatFromEuler( const Vec3& degrees );
Vec3 EulerFromQuat( const Quat& rotation );