#include "gaia3_pch.h"
#include "lightComponent.h"

#include <scene/entity.h>

RTTI_DEFINE_COMP( DirectionalLightComponent,
                  RTTI_PROPERTY( m_Color ),
                  RTTI_PROPERTY( m_Intensity ) )

RTTI_DEFINE_COMP( PointLightComponent,
                  RTTI_PROPERTY( m_Color ),
                  RTTI_PROPERTY( m_Intensity ),
                  RTTI_PROPERTY( m_Range ) )

LightParameters DirectionalLightComponent::GetLightParameters() const
{
	LightParameters light;
	light.type      = LightType::Directional;
	light.color     = m_Color * Scalar( m_Intensity );
	light.direction = GetOwner()->GetTransform().rotation.Normalized().Rotate( Vec3::V_Front );
	return light;
}

LightParameters PointLightComponent::GetLightParameters() const
{
	LightParameters light;
	light.type     = LightType::Point;
	light.color    = m_Color * Scalar( m_Intensity );
	light.position = GetOwner()->GetTransform().position;
	light.range    = m_Range;
	return light;
}
