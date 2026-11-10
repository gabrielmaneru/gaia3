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

	void OnTransformChanged() override;
	void DrawInspector() override;

	Matrix44 GetProjectionMatrix( rect canvas ) const override;
	Matrix44 GetViewMatrix() const override;

	float GetYaw() const { return m_Yaw; }
	float GetPitch() const { return m_Pitch; }
	void  SetRotationAngles( float yaw, float pitch );

	// Note: Shared by every camera and not saved in levels, the editor changes them from its Settings menu
	static float GetLinearSpeed() { return m_LinearSpeed; }
	static float GetAngularSpeed() { return m_AngularSpeed; }
	static void  SetLinearSpeed( float speed ) { m_LinearSpeed = speed; }
	static void  SetAngularSpeed( float speed ) { m_AngularSpeed = speed; }

	static constexpr float kDefaultLinearSpeed  = 0.01f;
	static constexpr float kDefaultAngularSpeed = 0.2f; // deg

private:
	void ComputeAnglesFromRotation();
	void RecomputeRotation();

	float m_HorizontalFOV{ 90.0f }; // deg
	float m_NearPlane{ 0.01f };
	float m_FarPlane{ 100.0f };

	// Note: Runtime only, the saved orientation is the entity's rotation
	float m_Yaw{ 0.0f };   // deg
	float m_Pitch{ 0.0f }; // deg
	// Assume m_Roll = 0.0f

	// Note: Units per frame and degrees per pixel of mouse movement
	static float m_LinearSpeed;
	static float m_AngularSpeed;
};
