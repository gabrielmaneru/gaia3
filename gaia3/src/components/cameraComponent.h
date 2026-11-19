#pragma once

#include <gfx/rendererTypes.h>
#include <math/boundingBox.h>
#include <scene/componentFactory.h>

// Note: What every camera shares: a perspective lens, joining the renderer, and seeing from its entity's transform.
// It isn't a component on its own, the camera types derive from it, move their entity their own way, and list
// m_HorizontalFOV, m_NearPlane and m_FarPlane among their RTTI properties when they use them
class CameraComponent : public Component, public RenderCamera
{
public:
	void OnAddComponent() override { RenderCamera::Register(); }
	void OnRemoveComponent() override { RenderCamera::Unregister(); }

	Matrix44 GetProjectionMatrix( rect canvas ) const override;
	Matrix44 GetViewMatrix() const override;

	float GetHorizontalFOV() const { return m_HorizontalFOV; }
	void  SetHorizontalFOV( float degrees ) { m_HorizontalFOV = degrees; }
	float GetNearPlane() const { return m_NearPlane; }
	void  SetNearPlane( float distance ) { m_NearPlane = distance; }
	float GetFarPlane() const { return m_FarPlane; }
	void  SetFarPlane( float distance ) { m_FarPlane = distance; }

protected:
	// Note: Only the camera types can be created, this is their shared base
	CameraComponent() = default;

	// Note: The planes the projection uses, the saved ones unless a camera type works them out itself
	virtual float GetProjectionNearPlane() const { return m_NearPlane; }
	virtual float GetProjectionFarPlane() const { return m_FarPlane; }

	float m_HorizontalFOV = 90.0f; // deg
	float m_NearPlane     = 0.01f;
	float m_FarPlane      = 100.0f;
};

// Note: A camera that flies freely, holding the right mouse button looks around and WASD moves, Q and E go down and
// up, Shift goes faster. The sandbox's camera, in the game and in the editor
class FreeCameraComponent : public CameraComponent
{
	RTTI_DECLARE_COMP( FreeCameraComponent, CameraComponent )
public:
	FreeCameraComponent() = default;

	void OnAddComponent() override;
	void Update() override;

	void OnTransformChanged() override;
	void DrawInspector() override;

	float GetYaw() const { return m_Yaw; }
	float GetPitch() const { return m_Pitch; }
	void  SetRotationAngles( float yaw, float pitch );

	// Note: Shared by every free camera and not saved in levels, the editor changes them from its Settings menu
	static float GetLinearSpeed() { return m_LinearSpeed; }
	static float GetAngularSpeed() { return m_AngularSpeed; }
	static void  SetLinearSpeed( float speed ) { m_LinearSpeed = speed; }
	static void  SetAngularSpeed( float speed ) { m_AngularSpeed = speed; }

	static constexpr float kDefaultLinearSpeed  = 0.01f;
	static constexpr float kDefaultAngularSpeed = 0.2f; // deg

private:
	void ComputeAnglesFromRotation();
	void RecomputeRotation();

	// Note: Runtime only, the saved orientation is the entity's rotation
	float m_Yaw{ 0.0f };   // deg
	float m_Pitch{ 0.0f }; // deg
	// Assume m_Roll = 0.0f

	// Note: Units per frame and degrees per pixel of mouse movement
	static float m_LinearSpeed;
	static float m_AngularSpeed;
};

// Note: A camera that turns around a target point, for looking at one object. Right drag orbits, middle drag pans
// and the wheel zooms. It sets its entity's transform every frame, so it is the only thing that moves it. Its near and
// far planes follow its distance, so tiny and huge models both keep their depth precision
class OrbitCameraComponent : public CameraComponent
{
	RTTI_DECLARE_COMP( OrbitCameraComponent, CameraComponent )
public:
	OrbitCameraComponent() = default;

	void OnAddComponent() override;
	void Update() override;

	// Note: Looks at the bounding volume from the current direction, from far enough to see all of it
	void Frame( const AABB& boundingVolume );

	const Vec3& GetTarget() const { return m_Target; }
	float       GetDistance() const { return m_Distance; }
	float       GetYaw() const { return m_Yaw; }
	float       GetPitch() const { return m_Pitch; }

	// Note: Yaw turns around the target, 0 looks along -Z, and negative pitch looks down
	void SetOrbit( const Vec3& target, float distance, float yaw, float pitch );

protected:
	float GetProjectionNearPlane() const override { return m_Distance * 0.001f; }
	float GetProjectionFarPlane() const override { return m_Distance * 100.0f; }

private:
	void ApplyToOwner();

	Vec3  m_Target   = Vec3::V_Zero;
	float m_Distance = 5.0f;
	float m_Yaw      = 35.0f;  // deg
	float m_Pitch    = -25.0f; // deg

	// Note: Degrees per pixel of mouse movement, and how much each wheel step brings the camera closer
	static constexpr float kOrbitSpeed = 0.3f;
	static constexpr float kZoomStep   = 0.85f;
};
