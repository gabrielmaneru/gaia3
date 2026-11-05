#include "gaia3_pch.h"
#include "quaternionTransforms.h"

#include <math/matrixTransforms.h>

#include <math.h>

float Dot( const Quat& a, const Quat& b )
{
	return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) + ( a.w * b.w );
}

Quat QuatFromMatrix33( const Matrix33& m )
{
	const float trace = m.m00 + m.m11 + m.m22;

	Quat q;
	if( trace > 0.0f )
	{
		const float s = sqrtf( trace + 1.0f ) * 2.0f;
		q.w           = 0.25f * s;
		q.x           = ( m.m21 - m.m12 ) / s;
		q.y           = ( m.m02 - m.m20 ) / s;
		q.z           = ( m.m10 - m.m01 ) / s;
	}
	else if( m.m00 > m.m11 && m.m00 > m.m22 )
	{
		const float s = sqrtf( 1.0f + m.m00 - m.m11 - m.m22 ) * 2.0f;
		q.w           = ( m.m21 - m.m12 ) / s;
		q.x           = 0.25f * s;
		q.y           = ( m.m01 + m.m10 ) / s;
		q.z           = ( m.m02 + m.m20 ) / s;
	}
	else if( m.m11 > m.m22 )
	{
		const float s = sqrtf( 1.0f + m.m11 - m.m00 - m.m22 ) * 2.0f;
		q.w           = ( m.m02 - m.m20 ) / s;
		q.x           = ( m.m01 + m.m10 ) / s;
		q.y           = 0.25f * s;
		q.z           = ( m.m12 + m.m21 ) / s;
	}
	else
	{
		const float s = sqrtf( 1.0f + m.m22 - m.m00 - m.m11 ) * 2.0f;
		q.w           = ( m.m10 - m.m01 ) / s;
		q.x           = ( m.m02 + m.m20 ) / s;
		q.y           = ( m.m12 + m.m21 ) / s;
		q.z           = 0.25f * s;
	}

	return q.Normalized();
}

Quat LookAtQuat( const Vec3& forwardDir, const Vec3& upAxis )
{
	const Matrix33 lookAt = LookAt33( forwardDir, upAxis );
	return QuatFromMatrix33( lookAt );
}