#include "gaia3_pch.h"
#include "lightComponent.h"

#include <core/app.h>
#include <scene/entity.h>
#include <scene/inspector.h>

RTTI_DEFINE_COMP( DirectionalLightComponent,
                  RTTI_PROPERTY( m_Color ),
                  RTTI_PROPERTY( m_Intensity ),
                  RTTI_PROPERTY( m_bCastShadows ),
                  RTTI_PROPERTY( m_ShadowSize ),
                  RTTI_PROPERTY( m_ShadowDepth ) )

RTTI_DEFINE_COMP( PointLightComponent,
                  RTTI_PROPERTY( m_Color ),
                  RTTI_PROPERTY( m_Intensity ),
                  RTTI_PROPERTY( m_Range ) )

LightParameters DirectionalLightComponent::GetLightParameters() const
{
	LightParameters light;
	light.type      = LightType::Directional;
	light.color     = m_Color * Scalar( m_Intensity );
	light.direction    = GetOwner()->GetTransform().rotation.Normalized().Rotate( Vec3::V_Front );
	light.position     = GetOwner()->GetTransform().position;
	light.bCastShadows = m_bCastShadows;
	light.shadowSize   = m_ShadowSize;
	light.shadowDepth  = m_ShadowDepth;
	return light;
}

void DirectionalLightComponent::DrawInspector()
{
	Component::DrawInspector();

	const Renderer& renderer = App::GetRenderer();
	if( renderer.GetShadowCaster() == this )
	{
		Inspector::ShowNote( "Shadow map" );
		Inspector::ShowDepthTexture( renderer.GetShadowMapTexture(), 256.0f );
	}
	else if( !m_bCastShadows )
	{
		Inspector::ShowNote( "No shadow map, CastShadows is off" );
	}
	else if( !renderer.GetShadowSettings().bEnabled )
	{
		Inspector::ShowNote( "No shadow map, sun shadows are off in Settings" );
	}
	else
	{
		Inspector::ShowNote( "No shadow map, another directional light has it" );
	}
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
