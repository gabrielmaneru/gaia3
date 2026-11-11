#include "testHelpers.h"

#include <components/lightComponent.h>
#include <core/app.h>
#include <math/quaternionTransforms.h>
#include <scene/level.h>

#include <algorithm>

namespace
{
bool IsRegistered( const RenderLight* light )
{
	const std::vector<RenderLight*>& lights = App::GetRenderer().GetLights();
	return std::find( lights.begin(), lights.end(), light ) != lights.end();
}

void ExpectNear( const Vec3& actual, const Vec3& expected )
{
	EXPECT_NEAR( actual.x, expected.x, 1e-4f );
	EXPECT_NEAR( actual.y, expected.y, 1e-4f );
	EXPECT_NEAR( actual.z, expected.z, 1e-4f );
}
} // namespace

TEST( LightComponentTests, BothLightTypesAreRegisteredComponents )
{
	const std::vector<std::string> names = ComponentFactory::GetRegisteredTypeNames();

	EXPECT_NE( std::find( names.begin(), names.end(), "DirectionalLightComponent" ), names.end() );
	EXPECT_NE( std::find( names.begin(), names.end(), "PointLightComponent" ), names.end() );
}

TEST( LightComponentTests, LightsJoinAndLeaveTheRenderer )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Light" );

	shared<PointLightComponent> light = entity->AddComponent<PointLightComponent>();
	EXPECT_TRUE( IsRegistered( light.get() ) );

	entity->RemoveComponent( light.get() );
	EXPECT_FALSE( IsRegistered( light.get() ) );
}

TEST( LightComponentTests, DirectionalLightShinesAlongItsEntitysForward )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Sun" );

	const Vec3 direction = Vec3( -0.4f, -0.8f, -0.6f ).Normalized();
	entity->SetRotation( LookAtQuat( direction, Vec3::V_Up ) );

	shared<DirectionalLightComponent> sun = entity->AddComponent<DirectionalLightComponent>();
	sun->SetColor( Vec3( 1.0f, 0.5f, 0.25f ) );
	sun->SetIntensity( 2.0f );

	const LightParameters light = sun->GetLightParameters();
	EXPECT_EQ( light.type, LightType::Directional );
	ExpectNear( light.direction, direction );
	ExpectNear( light.color, Vec3( 2.0f, 1.0f, 0.5f ) );
}

TEST( LightComponentTests, OnlyDirectionalLightsCastShadows )
{
	Level   level;
	Entity* sunEntity  = level.CreateEntity( "Sun" );
	Entity* lampEntity = level.CreateEntity( "Lamp" );

	shared<DirectionalLightComponent> sun = sunEntity->AddComponent<DirectionalLightComponent>();
	EXPECT_TRUE( sun->GetCastShadows() );
	EXPECT_TRUE( sun->GetLightParameters().bCastShadows );

	sun->SetCastShadows( false );
	EXPECT_FALSE( sun->GetLightParameters().bCastShadows );

	EXPECT_FALSE( lampEntity->AddComponent<PointLightComponent>()->GetLightParameters().bCastShadows );
}

TEST( LightComponentTests, ShadowSettingsAreSaved )
{
	Level   source;
	Entity* sourceEntity = source.CreateEntity( "Sun" );

	shared<DirectionalLightComponent> sun = sourceEntity->AddComponent<DirectionalLightComponent>();
	sun->SetCastShadows( false );
	sun->SetShadowSize( 46.0f );
	sun->SetShadowDepth( 60.0f );

	Json::Value json;
	ASSERT_TRUE( sourceEntity->SerializeComponents( json ) );
	EXPECT_FALSE( json[0]["m_Data"]["m_bCastShadows"].asBool() );
	EXPECT_FLOAT_EQ( json[0]["m_Data"]["m_ShadowSize"].asFloat(), 46.0f );

	Level   target;
	Entity* targetEntity = target.CreateEntity( "Sun" );
	ASSERT_TRUE( targetEntity->DeserializeComponents( json ) );

	const DirectionalLightComponent* loaded = targetEntity->GetComponent<DirectionalLightComponent>();
	EXPECT_FALSE( loaded->GetCastShadows() );
	EXPECT_FLOAT_EQ( loaded->GetShadowSize(), 46.0f );
	EXPECT_FLOAT_EQ( loaded->GetShadowDepth(), 60.0f );
}

TEST( LightComponentTests, TheShadowBoxIsCentredOnTheSun )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Sun" );
	entity->SetPosition( Vec3( -0.6f, 6.5f, -0.4f ) );

	shared<DirectionalLightComponent> sun = entity->AddComponent<DirectionalLightComponent>();
	sun->SetShadowSize( 46.0f );
	sun->SetShadowDepth( 50.0f );

	const LightParameters light = sun->GetLightParameters();
	ExpectNear( light.position, Vec3( -0.6f, 6.5f, -0.4f ) );
	EXPECT_FLOAT_EQ( light.shadowSize, 46.0f );
	EXPECT_FLOAT_EQ( light.shadowDepth, 50.0f );
}

TEST( LightComponentTests, DirectionalLightWithoutRotationShinesForward )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Sun" );

	const LightParameters light = entity->AddComponent<DirectionalLightComponent>()->GetLightParameters();

	ExpectNear( light.direction, Vec3::V_Front );
}

TEST( LightComponentTests, PointLightSitsAtItsEntity )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Lamp" );
	entity->SetPosition( Vec3( -6.0f, 2.2f, 4.1f ) );

	shared<PointLightComponent> lamp = entity->AddComponent<PointLightComponent>();
	lamp->SetColor( Vec3( 1.0f, 0.55f, 0.25f ) );
	lamp->SetIntensity( 4.0f );
	lamp->SetRange( 8.0f );

	const LightParameters light = lamp->GetLightParameters();
	EXPECT_EQ( light.type, LightType::Point );
	ExpectNear( light.position, Vec3( -6.0f, 2.2f, 4.1f ) );
	ExpectNear( light.color, Vec3( 4.0f, 2.2f, 1.0f ) );
	EXPECT_FLOAT_EQ( light.range, 8.0f );

	entity->SetPosition( Vec3( 1.0f, 2.0f, 3.0f ) );
	ExpectNear( lamp->GetLightParameters().position, Vec3( 1.0f, 2.0f, 3.0f ) );
}

TEST( LightComponentTests, LightsSaveAndLoadTheirProperties )
{
	Level   source;
	Entity* sourceEntity = source.CreateEntity( "Lamp" );

	shared<PointLightComponent> lamp = sourceEntity->AddComponent<PointLightComponent>();
	lamp->SetColor( Vec3( 0.25f, 0.5f, 1.0f ) );
	lamp->SetIntensity( 3.0f );
	lamp->SetRange( 12.0f );

	Json::Value json;
	ASSERT_TRUE( sourceEntity->SerializeComponents( json ) );
	EXPECT_EQ( json[0]["m_TypeName"].asString(), "PointLightComponent" );
	EXPECT_FLOAT_EQ( json[0]["m_Data"]["m_Range"].asFloat(), 12.0f );

	Level   target;
	Entity* targetEntity = target.CreateEntity( "Lamp" );
	ASSERT_TRUE( targetEntity->DeserializeComponents( json ) );

	const PointLightComponent* loaded = targetEntity->GetComponent<PointLightComponent>();
	ASSERT_NE( loaded, nullptr );
	ExpectNear( loaded->GetColor(), Vec3( 0.25f, 0.5f, 1.0f ) );
	EXPECT_FLOAT_EQ( loaded->GetIntensity(), 3.0f );
	EXPECT_FLOAT_EQ( loaded->GetRange(), 12.0f );
}

TEST( LightComponentTests, TheRendererUsesAtMostTheLightLimit )
{
	Level      level;
	LogCapture log;

	for( u32 i = 0; i < kMaxLights + 2; ++i )
	{
		level.CreateEntity( "Lamp" )->AddComponent<PointLightComponent>();
	}

	EXPECT_EQ( App::GetRenderer().GatherLights().size(), kMaxLights );
	EXPECT_TRUE( log.Contains( "only the first 8 are used" ) );

	// Note: The warning is only given once while the limit stays passed
	LogCapture secondFrame;
	App::GetRenderer().GatherLights();
	EXPECT_FALSE( secondFrame.Contains( "only the first" ) );
}
