#include "gaia3_pch.h"
#include "matrixTransforms.h"

#include <math.h>

Matrix33 LookAt33( const Vec3& forwardDir, const Vec3& upAxis )
{
	// Build a right-handed orthonormal basis from the input direction and
	// world-up.
	// - front points toward the camera look direction.
	// - right is perpendicular to front and up.
	// - up is recomputed to ensure orthogonality.

	Vec3 front = forwardDir.Normalized();
	if( front == Vec3::V_Zero )
	{
		front = Vec3::V_Front;
	}

	Vec3 upAxisNorm = upAxis.Normalized();
	if( upAxisNorm == Vec3::V_Zero )
	{
		upAxisNorm = Vec3::V_Up;
	}

	Vec3 right = Cross( front, upAxisNorm ).Normalized();
	if( right == Vec3::V_Zero )
	{
		// front and upAxis are parallel; choose a fallback axis to avoid
		// degeneracy.
		if( IsClose( front, Vec3::V_Up, eps_6 ) || IsClose( front, -Vec3::V_Up, eps_6 ) )
		{
			right = Cross( front, Vec3::V_Right ).Normalized();
		}
		else
		{
			right = Cross( front, Vec3::V_Up ).Normalized();
		}
	}

	const Vec3 up = Cross( right, front ).Normalized();

	// Note: The axes are columns, so the local axes map to world directions and V_Front maps to front
	return Matrix33( right, up, -front );
}

Matrix44 Perspective( float fovRadians, float aspectRatio, float nearPlane, float farPlane )
{
	// Compute a right-handed perspective projection matrix.
	// - Input FOV is expected in radians (vertical FOV).
	// - Matrix layout is column-major with column vectors.
	// - NDC Z range is [-1, 1] (OpenGL convention).

	const float clampedAspect = ( aspectRatio <= eps_6 ) ? 1.0f : aspectRatio;
	const float clampedNear   = ( nearPlane <= eps_6 ) ? 0.01f : nearPlane;
	const float clampedFar    = ( farPlane <= clampedNear ) ? ( clampedNear + 0.01f ) : farPlane;
	if( clampedAspect != aspectRatio || clampedNear != nearPlane || clampedFar != farPlane )
	{
		GAIA_WARN( "Perspective: invalid aspect {}, near {} or far {}, using clamped values", aspectRatio, nearPlane, farPlane );
	}

	const float tanHalfFovY = tanf( fovRadians * 0.5f );

	Matrix44 projection;
	projection.SetRow( 0, Vec4( 1.0f / ( tanHalfFovY * clampedAspect ), 0.0f, 0.0f, 0.0f ) );
	projection.SetRow( 1, Vec4( 0.0f, 1.0f / tanHalfFovY, 0.0f, 0.0f ) );
	projection.SetRow( 2, Vec4( 0.0f, 0.0f, ( clampedFar + clampedNear ) / ( clampedNear - clampedFar ), ( 2.0f * clampedFar * clampedNear ) / ( clampedNear - clampedFar ) ) );
	projection.SetRow( 3, Vec4( 0.0f, 0.0f, -1.0f, 0.0f ) );

	return projection;
}

Matrix44 Orthographic( float left, float right, float bottom, float top, float nearPlane, float farPlane )
{
	GAIA_ASSERT( right != left && top != bottom && farPlane != nearPlane );

	Matrix44 projection;
	projection.SetRow( 0, Vec4( 2.0f / ( right - left ), 0.0f, 0.0f, -( right + left ) / ( right - left ) ) );
	projection.SetRow( 1, Vec4( 0.0f, 2.0f / ( top - bottom ), 0.0f, -( top + bottom ) / ( top - bottom ) ) );
	projection.SetRow( 2, Vec4( 0.0f, 0.0f, -2.0f / ( farPlane - nearPlane ), -( farPlane + nearPlane ) / ( farPlane - nearPlane ) ) );
	projection.SetRow( 3, Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	return projection;
}
