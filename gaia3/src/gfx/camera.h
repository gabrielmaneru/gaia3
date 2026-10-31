#pragma once

class Camera
{
public:
	void Init( rect canvas );
	void Update( rect canvas );

	const Matrix44& GetViewMatrix() const { return m_ViewMat; }
	const Matrix44& GetProjectionMatrix() const { return m_ProjectionMat; }

private:
	float m_HorizontalFOV{ 90.0f }; // deg
	float m_NearPlane{ 0.01f };
	float m_FarPlane{ 100.0f };

	float m_Yaw{ 90.0f };  // deg
	float m_Pitch{ 0.0f }; // deg
	// Assume m_Roll = 0.0f

	Vec3  m_CameraPos{ 0, 0, -4.12f };
	float m_LinearSpeed{ 0.05f };
	float m_AngularSpeed{ 0.2f }; // deg

	Vec3 m_FrontDir{ Vec3::V_Front };
	Vec3 m_RightDir{ Vec3::V_Right };
	Vec3 m_UpDir{ Vec3::V_Up };

	rect m_LastCanvas{ 0, 0 };

	Matrix44 m_ProjectionMat;
	Matrix44 m_ViewMat;

	void RecalculateBasisVectors();
	void RecalculateMatrices( rect canvas );
};
