#include "gaia3_pch.h"
#include "camera.h"

#include <platform/input.h>

void Camera::Init( rect canvas )
{
	RecalculateBasisVectors();
	RecalculateMatrices( canvas );
}

void Camera::Update( rect canvas )
{
	bool hasOrientationChanged = false;
	bool hasPositionChanged    = false;

	// Handle camera input
	if( input->IsMouseButtonDown( eInputMouseButtons::RIGHT ) )
	{
		Vec3 moveDir = Vec3::V_Zero;

		if( input->IsMouseMoving() )
		{
			const float mouseDeltaX = input->GetMouseDeltaX();
			const float mouseDeltaY = input->GetMouseDeltaY();

			m_Yaw += mouseDeltaX * m_AngularSpeed;
			m_Pitch += mouseDeltaY * m_AngularSpeed;
			m_Pitch = Clamp( m_Pitch, -89.0f, 89.0f );

			RecalculateBasisVectors();
			hasOrientationChanged = true;
		}

		if( input->IsKeyDown( eInputKeys::W ) )
			moveDir += m_FrontDir;
		if( input->IsKeyDown( eInputKeys::S ) )
			moveDir -= m_FrontDir;
		if( input->IsKeyDown( eInputKeys::D ) )
			moveDir += m_RightDir;
		if( input->IsKeyDown( eInputKeys::A ) )
			moveDir -= m_RightDir;
		if( input->IsKeyDown( eInputKeys::E ) )
			moveDir += m_UpDir;
		if( input->IsKeyDown( eInputKeys::Q ) )
			moveDir -= m_UpDir;

		if( moveDir != Vec3::V_Zero )
		{
			moveDir.Normalize();

			const float  shiftRamp = input->IsKeyDown( eInputKeys::LEFT_SHIFT ) ? 5.0f : 1.0f;
			const Scalar linearSpeed( m_LinearSpeed * shiftRamp );
			m_CameraPos += moveDir * linearSpeed;
			hasPositionChanged = true;
		}
	}

	const bool hasCanvasChanged = ( canvas.x != m_LastCanvas.x ) || ( canvas.y != m_LastCanvas.y );
	if( hasCanvasChanged || hasOrientationChanged || hasPositionChanged )
	{
		RecalculateMatrices( canvas );
	}
}

void Camera::RecalculateBasisVectors()
{
	const float cosYaw   = cosf( ToRadians( m_Yaw ) );
	const float sinYaw   = sinf( ToRadians( m_Yaw ) );
	const float cosPitch = cosf( ToRadians( m_Pitch ) );
	const float sinPitch = sinf( ToRadians( m_Pitch ) );

	m_FrontDir = Vec3( cosYaw * cosPitch, sinPitch, sinYaw * cosPitch ).Normalized();
	m_RightDir = Cross( m_FrontDir, Vec3::V_Up ).Normalized();
	m_UpDir    = Cross( m_RightDir, m_FrontDir ).Normalized();
}

void Camera::RecalculateMatrices( rect canvas )
{
	assert( canvas.x > 0 && canvas.y > 0 );

	m_ViewMat = LookAt( m_CameraPos, m_FrontDir, Vec3::V_Up );

	const float aspectRatio          = (float)canvas.x / (float)canvas.y;
	const float horizontalFovRadians = ToRadians( m_HorizontalFOV );
	const float verticalFovRadians   = 2.0f * atanf( tanf( horizontalFovRadians * 0.5f ) / aspectRatio );

	m_ProjectionMat = Perspective( verticalFovRadians, aspectRatio, m_NearPlane, m_FarPlane );

	m_LastCanvas = canvas;
}
