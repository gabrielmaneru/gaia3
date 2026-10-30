#pragma once

class Camera
{
public:
	void Update(rect canvas);

	const Matrix44& GetViewMatrix() const { return m_ViewMat; }
	const Matrix44& GetProjectionMatrix() const { return m_ProjectionMat; }

private:
	float m_FOV{ 40.0f }; //deg
	float m_NearPlane{ 0.01f };
	float m_FarPlane{ 100.0f };

	float m_Yaw{ 90.0f }; //deg
	float m_Pitch{ 0.0f }; //deg
	// Assume m_Roll = 0.0f

	Vec3 m_CameraPos{ 0, 0, -4.12f };
	float m_LinearSpeed{ 0.05f };
	float m_AngularSpeed{ 0.2f }; //deg

	Matrix44 m_ProjectionMat;
	Matrix44 m_ViewMat;
	Matrix44 m_ViewProjectionMat;
};
