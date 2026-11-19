#include "cameraComponent.h"

#include <editor/inspector.h>
#include <math/mathHelpers.h>
#include <math/matrixTransforms.h>
#include <math/quaternionTransforms.h>
#include <math/vectorTransforms.h>
#include <platform/input.h>
#include <scene/componentFactory.h>
#include <scene/entity.h>

#include <math.h>

RTTI_DEFINE_COMP( FreeCameraComponent,
                  RTTI_PROPERTY( m_HorizontalFOV ),
                  RTTI_PROPERTY( m_NearPlane ),
                  RTTI_PROPERTY( m_FarPlane ) )

RTTI_DEFINE_COMP( OrbitCameraComponent,
                  RTTI_PROPERTY( m_HorizontalFOV ),
                  RTTI_PROPERTY( m_Target ),
                  RTTI_PROPERTY( m_Distance ),
                  RTTI_PROPERTY( m_Yaw ),
                  RTTI_PROPERTY( m_Pitch ) )

namespace
{
// Note: The direction the orbit camera looks in, from yaw and pitch in degrees, 0 yaw looks along -Z
Vec3 GetOrbitLookDirection( float yaw, float pitch )
{
	const float yawRad   = ToRadians( yaw );
	const float pitchRad = ToRadians( pitch );
	return Vec3( -sinf( yawRad ) * cosf( pitchRad ), sinf( pitchRad ), -cosf( yawRad ) * cosf( pitchRad ) );
}
} // namespace

Matrix44 CameraComponent::GetProjectionMatrix( rect canvas ) const
{
	const float aspectRatio          = static_cast<float>( canvas.x ) / static_cast<float>( canvas.y );
	const float horizontalFovRadians = ToRadians( m_HorizontalFOV );
	const float verticalFovRadians   = 2.0f * atanf( tanf( horizontalFovRadians * 0.5f ) / aspectRatio );

	return Perspective( verticalFovRadians, aspectRatio, GetProjectionNearPlane(), GetProjectionFarPlane() );
}

Matrix44 CameraComponent::GetViewMatrix() const
{
	return GetOwner()->GetTransform().Inverse().ToMatrix44();
}

float FreeCameraComponent::m_LinearSpeed  = FreeCameraComponent::kDefaultLinearSpeed;
float FreeCameraComponent::m_AngularSpeed = FreeCameraComponent::kDefaultAngularSpeed;

void FreeCameraComponent::OnAddComponent()
{
	CameraComponent::OnAddComponent();
	ComputeAnglesFromRotation();
}

void FreeCameraComponent::Update()
{
	if( input->IsMouseButtonDown( eInputMouseButtons::RIGHT ) )
	{
		// Camera rotation
		if( input->IsMouseMoving() )
		{
			const float mouseDeltaX = input->GetMouseDeltaX();
			const float mouseDeltaY = input->GetMouseDeltaY();

			SetRotationAngles( m_Yaw + mouseDeltaX * m_AngularSpeed, m_Pitch + mouseDeltaY * m_AngularSpeed );
		}

		// Camera translation
		{
			const Quat rot = GetOwner()->GetRotation();

			const Vec3 front = rot.Rotate( Vec3::V_Front );
			const Vec3 right = rot.Rotate( Vec3::V_Right );
			const Vec3 up    = rot.Rotate( Vec3::V_Up );

			Vec3 moveDir = Vec3::V_Zero;
			if( input->IsKeyDown( eInputKeys::W ) )
				moveDir += front;
			if( input->IsKeyDown( eInputKeys::S ) )
				moveDir -= front;
			if( input->IsKeyDown( eInputKeys::D ) )
				moveDir += right;
			if( input->IsKeyDown( eInputKeys::A ) )
				moveDir -= right;
			if( input->IsKeyDown( eInputKeys::E ) )
				moveDir += up;
			if( input->IsKeyDown( eInputKeys::Q ) )
				moveDir -= up;

			if( moveDir != Vec3::V_Zero )
			{
				moveDir.Normalize();

				const float  shiftMult = input->IsKeyDown( eInputKeys::LEFT_SHIFT ) ? 5.0f : 1.0f;
				const Scalar linearSpeed( m_LinearSpeed * shiftMult );
				GetOwner()->SetPosition( GetOwner()->GetPosition() + ( moveDir * linearSpeed ) );
			}
		}
	}
}

void FreeCameraComponent::OnTransformChanged()
{
	ComputeAnglesFromRotation();
}

void FreeCameraComponent::DrawInspector()
{
	float yaw   = m_Yaw;
	float pitch = m_Pitch;

	const bool bYawChanged   = Inspector::EditFloat( "Yaw", yaw, 0.5f );
	const bool bPitchChanged = Inspector::EditFloat( "Pitch", pitch, 0.5f );
	if( bYawChanged || bPitchChanged )
	{
		SetRotationAngles( yaw, pitch );
	}

	Component::DrawInspector();
}

void FreeCameraComponent::SetRotationAngles( float yaw, float pitch )
{
	m_Yaw   = yaw;
	m_Pitch = Clamp( pitch, -89.0f, 89.0f );

	RecomputeRotation();
}

void FreeCameraComponent::ComputeAnglesFromRotation()
{
	// Note: The entity's rotation is left untouched here, so a saved camera keeps it exactly until it is moved
	const Vec3 front = GetOwner()->GetRotation().Normalized().Rotate( Vec3::V_Front );

	m_Yaw   = ToDegrees( atan2f( front.z, front.x ) );
	m_Pitch = Clamp( ToDegrees( asinf( Clamp( front.y, -1.0f, 1.0f ) ) ), -89.0f, 89.0f );
}

void FreeCameraComponent::RecomputeRotation()
{
	const float cosYaw   = cosf( ToRadians( m_Yaw ) );
	const float sinYaw   = sinf( ToRadians( m_Yaw ) );
	const float cosPitch = cosf( ToRadians( m_Pitch ) );
	const float sinPitch = sinf( ToRadians( m_Pitch ) );

	const Vec3 front = Vec3( cosYaw * cosPitch, sinPitch, sinYaw * cosPitch ).Normalized();
	const Vec3 right = Cross( front, Vec3::V_Up ).Normalized();
	const Vec3 up    = Cross( right, front ).Normalized();

	GetOwner()->SetRotation( LookAtQuat( front, up ) );
}

void OrbitCameraComponent::OnAddComponent()
{
	CameraComponent::OnAddComponent();
	ApplyToOwner();
}

void OrbitCameraComponent::Update()
{
	if( input->IsMouseButtonDown( eInputMouseButtons::RIGHT ) )
	{
		m_Yaw -= input->GetMouseDeltaX() * kOrbitSpeed;
		m_Pitch = Clamp( m_Pitch + input->GetMouseDeltaY() * kOrbitSpeed, -89.0f, 89.0f );
	}

	// Note: Panning moves the target across the view, by an amount that keeps up with the mouse at the target's distance
	if( input->IsMouseButtonDown( eInputMouseButtons::MIDDLE ) )
	{
		const Quat  rotation = GetOwner()->GetRotation();
		const float speed    = m_Distance * 0.0015f;
		m_Target -= rotation.Rotate( Vec3::V_Right ) * Scalar( input->GetMouseDeltaX() * speed );
		m_Target -= rotation.Rotate( Vec3::V_Up ) * Scalar( input->GetMouseDeltaY() * speed );
	}

	const float wheel = input->GetMouseWheel();
	if( wheel != 0.0f )
	{
		m_Distance = Max( m_Distance * powf( kZoomStep, wheel ), 0.001f );
	}

	ApplyToOwner();
}

void OrbitCameraComponent::Frame( const AABB& boundingVolume )
{
	if( boundingVolume.IsEmpty() )
		return;

	// Note: Far enough for the sphere around the bounding volume to fit the vertical view, which is the narrower one for a viewport
	// about 16 by 10, with a small margin
	const float radius       = Max( boundingVolume.GetRadius(), 0.001f );
	const float halfVertical = atanf( tanf( ToRadians( m_HorizontalFOV * 0.5f ) ) / 1.6f );
	m_Target                 = boundingVolume.GetCenter();
	m_Distance               = radius / sinf( halfVertical ) * 1.05f;

	ApplyToOwner();
}

void OrbitCameraComponent::SetOrbit( const Vec3& target, float distance, float yaw, float pitch )
{
	m_Target   = target;
	m_Distance = Max( distance, 0.001f );
	m_Yaw      = yaw;
	m_Pitch    = Clamp( pitch, -89.0f, 89.0f );

	ApplyToOwner();
}

void OrbitCameraComponent::ApplyToOwner()
{
	if( !HasOwner() )
		return;

	const Vec3 lookDirection = GetOrbitLookDirection( m_Yaw, m_Pitch );
	GetOwner()->SetPosition( m_Target - lookDirection * Scalar( m_Distance ) );
	GetOwner()->SetRotation( LookAtQuat( lookDirection, Vec3::V_Up ) );
}
