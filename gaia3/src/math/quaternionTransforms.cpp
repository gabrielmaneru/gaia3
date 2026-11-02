#include "gaia3_pch.h"
#include "quaternionTransforms.h"

#include <math/matrixTransforms.h>

float Dot( const Quat& a, const Quat& b )
{
	return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) + ( a.w * b.w );
}

Quat QuatFromMatrix33( const Matrix33& m )
{
	const float m00 = m.col_0.x;
	const float m01 = m.col_1.x;
	const float m02 = m.col_2.x;
	const float m10 = m.col_0.y;
	const float m11 = m.col_1.y;
	const float m12 = m.col_2.y;
	const float m20 = m.col_0.z;
	const float m21 = m.col_1.z;
	const float m22 = m.col_2.z;

	const float trace = m00 + m11 + m22;

	Quat q;
	if( trace > 0.0f )
	{
		const float s = sqrtf( trace + 1.0f ) * 2.0f;
		q.w           = 0.25f * s;
		q.x           = ( m21 - m12 ) / s;
		q.y           = ( m02 - m20 ) / s;
		q.z           = ( m10 - m01 ) / s;
	}
	else if( m00 > m11 && m00 > m22 )
	{
		const float s = sqrtf( 1.0f + m00 - m11 - m22 ) * 2.0f;
		q.w           = ( m21 - m12 ) / s;
		q.x           = 0.25f * s;
		q.y           = ( m01 + m10 ) / s;
		q.z           = ( m02 + m20 ) / s;
	}
	else if( m11 > m22 )
	{
		const float s = sqrtf( 1.0f + m11 - m00 - m22 ) * 2.0f;
		q.w           = ( m02 - m20 ) / s;
		q.x           = ( m01 + m10 ) / s;
		q.y           = 0.25f * s;
		q.z           = ( m12 + m21 ) / s;
	}
	else
	{
		const float s = sqrtf( 1.0f + m22 - m00 - m11 ) * 2.0f;
		q.w           = ( m10 - m01 ) / s;
		q.x           = ( m02 + m20 ) / s;
		q.y           = ( m12 + m21 ) / s;
		q.z           = 0.25f * s;
	}

	return q.Normalized();
}

Quat LookAtQuat( const Vec3& forwardDir, const Vec3& upAxis )
{
	const Matrix33 lookAt = LookAt33( forwardDir, upAxis );
	return QuatFromMatrix33( lookAt );
}