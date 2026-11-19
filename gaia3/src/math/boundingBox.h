#pragma once

#include <math/vector.h>

#include <cfloat>

// Note: An axis-aligned bounding box around points. It starts empty, inside out from the largest float to the smallest,
// so the first point added sets both corners. Oriented boxes (OBB) will live in this file too
struct AABB
{
	Vec3 min = Vec3( Scalar( FLT_MAX ) );
	Vec3 max = Vec3( Scalar( -FLT_MAX ) );

	bool IsEmpty() const { return min.x > max.x || min.y > max.y || min.z > max.z; }

	void Add( const Vec3& point );
	void Add( const AABB& box );

	// Note: Zero for an empty box
	Vec3 GetCenter() const;
	Vec3 GetSize() const;

	// Note: Half the diagonal, the radius of a sphere around the box
	float GetRadius() const;
};
