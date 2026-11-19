#include "boundingBox.h"

#include <math/mathHelpers.h>

#include <cmath>

void AABB::Add( const Vec3& point )
{
	min = Min( min, point );
	max = Max( max, point );
}

void AABB::Add( const AABB& box )
{
	// Note: Corner against corner, so an empty box, inside out at the largest floats, changes nothing
	min = Min( min, box.min );
	max = Max( max, box.max );
}

Vec3 AABB::GetCenter() const
{
	return IsEmpty() ? Vec3::V_Zero : ( min + max ) * Scalar( 0.5f );
}

Vec3 AABB::GetSize() const
{
	return IsEmpty() ? Vec3::V_Zero : max - min;
}

float AABB::GetRadius() const
{
	return sqrtf( GetSize().LengthSq() ) * 0.5f;
}
