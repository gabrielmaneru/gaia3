#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>

class CameraComponent : public Component, public RenderCamera
{
	RTTI_DECLARE_COMP( CameraComponent, Component )
public:
	CameraComponent() = default;

	void OnAddComponent() override;
	void OnRemoveComponent() override;

	void Update() override;

	Matrix44 GetProjectionMatrix( rect canvas ) const override;
	Matrix44 GetViewMatrix() const override;

private:
	void RecomputeRotation();

	float m_HorizontalFOV{ 90.0f }; // deg
	float m_NearPlane{ 0.01f };
	float m_FarPlane{ 100.0f };

	float m_Yaw{ 90.0f };  // deg
	float m_Pitch{ 0.0f }; // deg
	// Assume m_Roll = 0.0f

	float m_LinearSpeed{ 0.05f };
	float m_AngularSpeed{ 0.2f }; // deg
};
