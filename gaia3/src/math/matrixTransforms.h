#pragma once

Matrix44 LookAt( const Vec3& eye, const Vec3& forwardDir, const Vec3& upAxis );
Matrix44 Perspective( float fovRadians, float aspectRatio, float nearPlane, float farPlane );
