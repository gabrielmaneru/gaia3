#include "gaia3_pch.h"
#include "transform.h"

const Transform Transform::V_Identity( Vec3::V_Zero, Quat::V_Identity, Vec3::V_One );

Transform Transform::operator*( const Transform& rhs ) const
{
	Transform result( *this );
	return result *= rhs;
}

Transform& Transform::operator*=( const Transform& rhs )
{
	const Transform lhs( *this );

	const Quat lhsRotation = lhs.rotation.Normalized();
	const Vec3 scaledPos   = lhs.scale * rhs.position;
	const Vec3 rotatedPos  = lhsRotation.Rotate( scaledPos );

	position = lhs.position + rotatedPos;
	rotation = ( lhs.rotation * rhs.rotation ).Normalized();
	scale    = lhs.scale * rhs.scale;

	return *this;
}

Matrix44 Transform::ToMatrix44() const
{
	const Quat q = rotation.Normalized();

	const float xx = q.x * q.x;
	const float yy = q.y * q.y;
	const float zz = q.z * q.z;
	const float xy = q.x * q.y;
	const float xz = q.x * q.z;
	const float yz = q.y * q.z;
	const float wx = q.w * q.x;
	const float wy = q.w * q.y;
	const float wz = q.w * q.z;

	Vec3 col0( 1.0f - 2.0f * ( yy + zz ),
	           2.0f * ( xy + wz ),
	           2.0f * ( xz - wy ) );
	Vec3 col1( 2.0f * ( xy - wz ),
	           1.0f - 2.0f * ( xx + zz ),
	           2.0f * ( yz + wx ) );
	Vec3 col2( 2.0f * ( xz + wy ),
	           2.0f * ( yz - wx ),
	           1.0f - 2.0f * ( xx + yy ) );

	col0 *= Scalar( scale.x );
	col1 *= Scalar( scale.y );
	col2 *= Scalar( scale.z );

	return Matrix44( Vec4( col0, 0.0f ),
	                 Vec4( col1, 0.0f ),
	                 Vec4( col2, 0.0f ),
	                 Vec4( position, 1.0f ) );
}
