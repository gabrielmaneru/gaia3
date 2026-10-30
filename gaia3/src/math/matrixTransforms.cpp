#include "gaia3_pch.h"
#include "matrixTransforms.h"

#include <math.h>

Matrix44 LookAt(const Vec3& eye, const Vec3& forwardDir, const Vec3& upAxis)
{
	// Build a right-handed orthonormal basis from the input direction and world-up.
	// - front points toward the camera look direction.
	// - right is perpendicular to front and up.
	// - up is recomputed to ensure orthogonality.

	Vec3 front = forwardDir.Normalized();
	if (front == Vec3::V_Zero)
	{
		front = Vec3::V_Front;
	}
	Vec3 upAxisNorm	= upAxis.Normalized();
	if (upAxisNorm == Vec3::V_Zero)
	{
		upAxisNorm = Vec3::V_Up;
	}

	Vec3 right = Cross(front, upAxisNorm).Normalized();
	if (right == Vec3::V_Zero)
	{
		// front and upAxis are parallel; choose a fallback axis to avoid degeneracy.
		if (IsClose(front, Vec3::V_Up, eps_6) || IsClose(front, -Vec3::V_Up, eps_6))
		{
			right = Cross(front, Vec3::V_Right).Normalized();
		}
		else
		{
			right = Cross(front, Vec3::V_Up).Normalized();
		}
	}

	const Vec3 up = Cross(right, front).Normalized();

	// View translation terms from dot products with camera basis vectors.
	const float tx = -Dot(right, eye);
	const float ty = -Dot(up, eye);
	const float tz = Dot(front, eye);

	Matrix44 view;
	view.SetRow(0, Vec4(right, tx));
	view.SetRow(1, Vec4(up, ty));
	view.SetRow(2, Vec4(-front, tz));
	view.SetRow(3, Vec4(0.0f, 0.0f, 0.0f, 1.0f));

	return view;
}

Matrix44 Perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane)
{
	// Compute a right-handed perspective projection matrix.
	// - Input FOV is expected in radians (vertical FOV).
	// - Matrix layout is column-major with column vectors.
	// - NDC Z range is [-1, 1] (OpenGL convention).

	const float clampedAspect = (aspectRatio <= eps_6) ? 1.0f : aspectRatio;
	const float clampedNear = (nearPlane <= eps_6) ? 0.01f : nearPlane;
	const float clampedFar = (farPlane <= clampedNear) ? (clampedNear + 0.01f) : farPlane;

	const float tanHalfFovY = tanf(fovRadians * 0.5f);
	const float f1 = 1.0f / (tanHalfFovY * clampedAspect);
	const float f2 = 1.0f / tanHalfFovY;

	Matrix44 projection;
	projection.SetRow(0, Vec4(f1, 0.0f, 0.0f, 0.0f));
	projection.SetRow(1, Vec4(0.0f, f2, 0.0f, 0.0f));
	projection.SetRow(2, Vec4(0.0f, 0.0f, (clampedFar + clampedNear) / (clampedNear - clampedFar), (2.0f * clampedFar * clampedNear) / (clampedNear - clampedFar)));
	projection.SetRow(3, Vec4(0.0f, 0.0f, -1.0f, 0.0f));

	return projection;
}
