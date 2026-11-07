#pragma once

#include <math/matrix.h>

// Note: An orientation in the world that points the local front axis along forwardDir
Matrix33 LookAt33( const Vec3& forwardDir, const Vec3& upAxis );

Matrix44 Perspective( float fovRadians, float aspectRatio, float nearPlane, float farPlane );
