#include "sunComponent.h"

#include <math/mathHelpers.h>
#include <math/quaternionTransforms.h>
#include <math/vectorTransforms.h>
#include <scene/entity.h>
#include <scene/inspector.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

RTTI_DEFINE_COMP( SunComponent,
                  RTTI_PROPERTY( m_Origin ),
                  RTTI_PROPERTY( m_Distance ),
                  RTTI_PROPERTY( m_Latitude ),
                  RTTI_PROPERTY( m_DayOfYear ),
                  RTTI_PROPERTY( m_Time ),
                  RTTI_PROPERTY( m_NorthAngle ) )

namespace
{
constexpr float kEarthTilt   = 23.44f; // deg
constexpr float kDaysPerYear = 365.0f;

// Note: North is -Z turned clockwise by northAngle degrees, seen from above
Vec3 GetNorth( float northAngle )
{
	const float northAngleRad = ToRadians( northAngle );
	return Vec3( sinf( northAngleRad ), 0.0f, -cosf( northAngleRad ) );
}
} // namespace

void SunComponent::OnAddComponent()
{
	ApplyToOwner();
}

void SunComponent::Update()
{
	// Note: Every frame, so moving the entity by hand doesn't stick
	ApplyToOwner();
}

void SunComponent::OnPropertyChanged( std::string_view propertyName )
{
	ApplyToOwner();
}

void SunComponent::DrawInspector()
{
	bool bChanged = Inspector::EditVec3( "Origin", m_Origin );
	bChanged |= Inspector::EditFloat( "Distance", m_Distance, 0.1f );
	bChanged |= Inspector::EditSlider( "Time", m_Time, 0.0f, 24.0f, "%.2f h" );
	bChanged |= Inspector::EditSlider( "Day of year", m_DayOfYear, 1.0f, kDaysPerYear, "%.0f" );
	bChanged |= Inspector::EditSlider( "Latitude", m_Latitude, -90.0f, 90.0f, "%.1f deg" );
	bChanged |= Inspector::EditSlider( "North angle", m_NorthAngle, 0.0f, 360.0f, "%.0f deg" );

	if( bChanged )
	{
		ApplyToOwner();
	}

	// Note: Where the sun is in the sky, the azimuth counts clockwise from north like a compass
	const Vec3  toSun     = ComputeSunDirection( m_Latitude, m_DayOfYear, m_Time, m_NorthAngle );
	const Vec3  north     = GetNorth( m_NorthAngle );
	const Vec3  east      = Cross( north, Vec3::V_Up );
	const float elevation = ToDegrees( asinf( Clamp( toSun.y, -1.0f, 1.0f ) ) );
	float       azimuth   = ToDegrees( atan2f( Dot( toSun, east ), Dot( toSun, north ) ) );
	if( azimuth < 0.0f )
	{
		azimuth += 360.0f;
	}

	const int         hours   = static_cast<int>( m_Time );
	const int         minutes = static_cast<int>( ( m_Time - static_cast<float>( hours ) ) * 60.0f );
	const std::string clock   = std::to_string( hours ) + ( minutes < 10 ? ":0" : ":" ) + std::to_string( minutes );
	const std::string note    = clock + ", elevation " + std::to_string( static_cast<int>( roundf( elevation ) ) ) + " deg, azimuth " + std::to_string( static_cast<int>( roundf( azimuth ) ) ) + " deg" + ( elevation < 0.0f ? ", below the horizon" : "" );
	Inspector::ShowNote( note.c_str() );
}

Vec3 SunComponent::ComputeSunDirection( float latitude, float dayOfYear, float time, float northAngle )
{
	// Note: The tilt of the Earth moves the sun north and south over the year, furthest north around 21 June
	const float declination = ToRadians( -kEarthTilt * cosf( 2.0f * static_cast<float>( M_PI ) / kDaysPerYear * ( dayOfYear + 10.0f ) ) );

	// Note: The Earth turns 15 degrees an hour, the sun is due south (north of the equator) at noon
	const float hourAngle   = ToRadians( 15.0f * ( time - 12.0f ) );
	const float latitudeRad = ToRadians( latitude );

	// Note: The sun's direction in east, north and up for the observer
	const float towardsEast  = -cosf( declination ) * sinf( hourAngle );
	const float towardsNorth = sinf( declination ) * cosf( latitudeRad ) - cosf( declination ) * cosf( hourAngle ) * sinf( latitudeRad );
	const float towardsUp    = sinf( declination ) * sinf( latitudeRad ) + cosf( declination ) * cosf( hourAngle ) * cosf( latitudeRad );

	const Vec3 north = GetNorth( northAngle );
	const Vec3 east  = Cross( north, Vec3::V_Up );

	return ( east * Scalar( towardsEast ) + north * Scalar( towardsNorth ) + Vec3::V_Up * Scalar( towardsUp ) ).Normalized();
}

void SunComponent::ApplyToOwner()
{
	if( !HasOwner() )
		return;

	const Vec3 toSun = ComputeSunDirection( m_Latitude, m_DayOfYear, m_Time, m_NorthAngle );

	GetOwner()->SetPosition( m_Origin + toSun * Scalar( m_Distance ) );
	GetOwner()->SetRotation( LookAtQuat( -toSun, Vec3::V_Up ) );
}
