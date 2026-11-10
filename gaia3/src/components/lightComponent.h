#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>

// Note: What every light shares, its colour, intensity and joining the renderer. It isn't a component on its own,
// the light types derive from it and list m_Color and m_Intensity among their RTTI properties
class LightComponent : public Component, public RenderLight
{
public:
	void OnAddComponent() override { RenderLight::Register(); }
	void OnRemoveComponent() override { RenderLight::Unregister(); }

	Vec3  GetColor() const { return m_Color; }
	void  SetColor( const Vec3& color ) { m_Color = color; }
	float GetIntensity() const { return m_Intensity; }
	void  SetIntensity( float intensity ) { m_Intensity = intensity; }

protected:
	Vec3  m_Color     = Vec3( 1.0f, 1.0f, 1.0f );
	float m_Intensity = 1.0f;
};

// Note: A faraway light like the sun, it shines along its entity's forward direction (-Z) from everywhere
class DirectionalLightComponent : public LightComponent
{
	RTTI_DECLARE_COMP( DirectionalLightComponent, LightComponent )
public:
	DirectionalLightComponent() = default;

	LightParameters GetLightParameters() const override;
};

// Note: A light at its entity's position shining every way, fading out to nothing at its range
class PointLightComponent : public LightComponent
{
	RTTI_DECLARE_COMP( PointLightComponent, LightComponent )
public:
	PointLightComponent() = default;

	LightParameters GetLightParameters() const override;

	float GetRange() const { return m_Range; }
	void  SetRange( float range ) { m_Range = range; }

private:
	float m_Range = 10.0f;
};
