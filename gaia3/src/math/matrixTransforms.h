#pragma once

#include <math/matrix.h>

Matrix33 LookAt33( const Vec3& forwardDir, const Vec3& upAxis );
Matrix44 LookAt44( const Vec3& eye, const Vec3& forwardDir, const Vec3& upAxis );

Matrix44 Perspective( float fovRadians, float aspectRatio, float nearPlane, float farPlane );
