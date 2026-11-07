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

Quat QuatFromEuler( const Vec3& degrees )
{
	const float halfX = ToRadians( degrees.x ) * 0.5f;
	const float halfY = ToRadians( degrees.y ) * 0.5f;
	const float halfZ = ToRadians( degrees.z ) * 0.5f;

	const Quat rotationX( sinf( halfX ), 0.0f, 0.0f, cosf( halfX ) );
	const Quat rotationY( 0.0f, sinf( halfY ), 0.0f, cosf( halfY ) );
	const Quat rotationZ( 0.0f, 0.0f, sinf( halfZ ), cosf( halfZ ) );

	return ( rotationY * rotationX * rotationZ ).Normalized();
}

Vec3 EulerFromQuat( const Quat& rotation )
{
	const Quat q = rotation.Normalized();

	const float m00 = 1.0f - 2.0f * ( q.y * q.y + q.z * q.z );
	const float m02 = 2.0f * ( q.x * q.z + q.w * q.y );
	const float m10 = 2.0f * ( q.x * q.y + q.w * q.z );
	const float m11 = 1.0f - 2.0f * ( q.x * q.x + q.z * q.z );
	const float m12 = 2.0f * ( q.y * q.z - q.w * q.x );
	const float m20 = 2.0f * ( q.x * q.z - q.w * q.y );
	const float m22 = 1.0f - 2.0f * ( q.x * q.x + q.y * q.y );

	// Note: Subtracting from zero avoids showing -0 for an unrotated axis
	const float angleX = asinf( Clamp( 0.0f - m12, -1.0f, 1.0f ) );

	// Note: Looking straight up or down, y and z spin around the same axis, so z is left at zero
	if( fabsf( m12 ) > 0.9999f )
	{
		return Vec3( ToDegrees( angleX ), ToDegrees( atan2f( -m20, m00 ) ), 0.0f );
	}

	return Vec3( ToDegrees( angleX ), ToDegrees( atan2f( m02, m22 ) ), ToDegrees( atan2f( m10, m11 ) ) );
}