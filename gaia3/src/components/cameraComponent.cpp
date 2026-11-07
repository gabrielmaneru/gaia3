#include "gaia3_pch.h"
#include "cameraComponent.h"

#include <platform/input.h>
#include <scene/componentFactory.h>
#include <scene/entity.h>
#include <scene/inspector.h>

#include <math.h>

RTTI_DEFINE_COMP( CameraComponent,
                  RTTI_PROPERTY( m_HorizontalFOV ),
                  RTTI_PROPERTY( m_NearPlane ),
                  RTTI_PROPERTY( m_FarPlane ),
                  RTTI_PROPERTY( m_LinearSpeed ),
                  RTTI_PROPERTY( m_AngularSpeed ) )

void CameraComponent::OnAddComponent()
{
	RenderCamera::Register();
	ComputeAnglesFromRotation();
}

void CameraComponent::OnRemoveComponent()
{
	RenderCamera::Unregister();
}

void CameraComponent::Update()
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

void CameraComponent::OnTransformChanged()
{
	ComputeAnglesFromRotation();
}

void CameraComponent::DrawInspector()
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

Matrix44 CameraComponent::GetProjectionMatrix( rect canvas ) const
{
	const float aspectRatio          = (float)canvas.x / (float)canvas.y;
	const float horizontalFovRadians = ToRadians( m_HorizontalFOV );
	const float verticalFovRadians   = 2.0f * atanf( tanf( horizontalFovRadians * 0.5f ) / aspectRatio );

	return Perspective( verticalFovRadians, aspectRatio, m_NearPlane, m_FarPlane );
}

Matrix44 CameraComponent::GetViewMatrix() const
{
	return GetOwner()->GetTransform().Inverse().ToMatrix44();
}

void CameraComponent::SetRotationAngles( float yaw, float pitch )
{
	m_Yaw   = yaw;
	m_Pitch = Clamp( pitch, -89.0f, 89.0f );

	RecomputeRotation();
}

void CameraComponent::ComputeAnglesFromRotation()
{
	// Note: The entity's rotation is left untouched here, so a saved camera keeps it exactly until it is moved
	const Vec3 front = GetOwner()->GetRotation().Normalized().Rotate( Vec3::V_Front );

	m_Yaw   = ToDegrees( atan2f( front.z, front.x ) );
	m_Pitch = Clamp( ToDegrees( asinf( Clamp( front.y, -1.0f, 1.0f ) ) ), -89.0f, 89.0f );
}

void CameraComponent::RecomputeRotation()
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
