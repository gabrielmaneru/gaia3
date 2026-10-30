#include "gaia3_pch.h"
#include "camera.h"

#include<core/input.h>

void Camera::Update(rect canvas)
{
	// Convert yaw/pitch (degrees) into a forward direction in world space.
	const float cosYaw = cosf(ToRadians(m_Yaw));
	const float sinYaw = sinf(ToRadians(m_Yaw));
	const float cosPitch = cosf(ToRadians(m_Pitch));
	const float sinPitch = sinf(ToRadians(m_Pitch));
	const Vec3 forwardDir(Vec3(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch).Normalized());

	// Note: use world-up as reference to preserve camera roll = 0.
	m_ViewMat = LookAt(m_CameraPos, forwardDir, Vec3::V_Up);

	assert(canvas.x > 0 && canvas.y > 0);
	const float aspectRatio = (float)canvas.x / (float)canvas.y;
	m_ProjectionMat = Perspective(ToRadians(m_FOV), aspectRatio, m_NearPlane, m_FarPlane);

	// Handle camera input
	if (input->IsMouseButtonDown(eInputMouseButtons::RIGHT))
	{
		Vec3 moveDir = Vec3::V_Zero;
		const Vec3 rightDir = Cross(forwardDir, Vec3::V_Up).Normalized();
		const Vec3 upDir = Cross(rightDir, forwardDir).Normalized();

		if (input->IsKeyDown(eInputKeys::W)) moveDir += forwardDir;
		if (input->IsKeyDown(eInputKeys::S)) moveDir -= forwardDir;
		if (input->IsKeyDown(eInputKeys::D)) moveDir += rightDir;
		if (input->IsKeyDown(eInputKeys::A)) moveDir -= rightDir;
		if (input->IsKeyDown(eInputKeys::E)) moveDir += upDir;
		if (input->IsKeyDown(eInputKeys::Q)) moveDir -= upDir;

		if (moveDir != Vec3::V_Zero)
		{
			moveDir.Normalize();

			const float shiftRamp = input->IsKeyDown(eInputKeys::LEFT_SHIFT) ? 5.0f : 1.0f;
			const Scalar linearSpeed(m_LinearSpeed * shiftRamp);
			m_CameraPos += moveDir * linearSpeed;
		}

		const float mouseDeltaX = static_cast<float>(input->GetMouseDeltaX());
		const float mouseDeltaY = static_cast<float>(input->GetMouseDeltaY());

		m_Yaw -= mouseDeltaX * m_AngularSpeed;
		m_Pitch -= mouseDeltaY * m_AngularSpeed;
		m_Pitch = Clamp(m_Pitch, -89.0f, 89.0f);
	}
}
