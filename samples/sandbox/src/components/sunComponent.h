#pragma once

#include <math/vector.h>
#include <scene/componentFactory.h>

// Note: Moves its entity along the sun's path through the sky around m_Origin, for a place on Earth at m_Latitude, a
// day of the year and a time of day. The entity sits m_Distance from the origin and looks at it, so a
// DirectionalLightComponent on the same entity shines like the sun, and its shadow box starts at the sun. With
// m_Distance about the level's radius, the light's ShadowSize and ShadowDepth should be about twice that. It overrides
// the entity's position and rotation every frame
class SunComponent : public Component
{
	RTTI_DECLARE_COMP( SunComponent, Component )
public:
	SunComponent() = default;

	void OnAddComponent() override;
	void Update() override;
	void OnPropertyChanged( std::string_view propertyName ) override;
	void DrawInspector() override;

	const Vec3& GetOrigin() const { return m_Origin; }
	void        SetOrigin( const Vec3& origin ) { m_Origin = origin; }
	float       GetDistance() const { return m_Distance; }
	void        SetDistance( float distance ) { m_Distance = distance; }
	float       GetLatitude() const { return m_Latitude; }
	void        SetLatitude( float latitude ) { m_Latitude = latitude; }
	float       GetDayOfYear() const { return m_DayOfYear; }
	void        SetDayOfYear( float dayOfYear ) { m_DayOfYear = dayOfYear; }
	float       GetTime() const { return m_Time; }
	void        SetTime( float time ) { m_Time = time; }
	float       GetNorthAngle() const { return m_NorthAngle; }
	void        SetNorthAngle( float northAngle ) { m_NorthAngle = northAngle; }

	// Note: The direction from the origin towards the sun, with Y up and north along -Z turned clockwise by northAngle
	// degrees, seen from above. Its Y is negative while the sun is below the horizon
	static Vec3 ComputeSunDirection( float latitude, float dayOfYear, float time, float northAngle );

private:
	void ApplyToOwner();

	Vec3 m_Origin = Vec3::V_Zero;

	// Note: In world units, how far the entity sits from the origin
	float m_Distance = 25.0f;

	// Note: Degrees, positive north of the equator
	float m_Latitude = 40.0f;

	// Note: 1 to 365, it sets how high the sun climbs, highest around day 172 in the north
	float m_DayOfYear = 172.0f;

	// Note: Solar time in hours, 0 to 24, the sun is highest at 12
	float m_Time = 15.0f;

	// Note: Degrees, turns the compass around the level, 0 has north along -Z and east along +X
	float m_NorthAngle = 0.0f;
};
