#include "testHelpers.h"

#include <components/cameraComponent.h>
#include <components/modelComponent.h>
#include <gfx/material.h>
#include <gfx/model.h>
#include <math/mathHelpers.h>
#include <math/quaternionTransforms.h>
#include <math/vectorTransforms.h>
#include <scene/level.h>

#include <math.h>

namespace
{
Vec3 FrontFromAngles( float yaw, float pitch )
{
	const float cosPitch = cosf( ToRadians( pitch ) );
	return Vec3( cosf( ToRadians( yaw ) ) * cosPitch, sinf( ToRadians( pitch ) ), sinf( ToRadians( yaw ) ) * cosPitch );
}
} // namespace

TEST( CameraComponentTests, AddingTheComponentKeepsTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	const Quat rotation = LookAtQuat( Vec3( 0.3f, 0.4f, 0.8f ).Normalized(), Vec3::V_Up );
	entity->SetRotation( rotation );

	entity->AddComponent<CameraComponent>();

	EXPECT_EQ( entity->GetRotation().x, rotation.x );
	EXPECT_EQ( entity->GetRotation().y, rotation.y );
	EXPECT_EQ( entity->GetRotation().z, rotation.z );
	EXPECT_EQ( entity->GetRotation().w, rotation.w );
}

TEST( CameraComponentTests, AnglesAreDerivedFromTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	entity->SetRotation( LookAtQuat( FrontFromAngles( 60.0f, -20.0f ), Vec3::V_Up ) );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	EXPECT_NEAR( camera->GetYaw(), 60.0f, 1e-2f );
	EXPECT_NEAR( camera->GetPitch(), -20.0f, 1e-2f );
}

TEST( CameraComponentTests, IdentityRotationLooksAlongTheFrontAxis )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	EXPECT_NEAR( camera->GetYaw(), -90.0f, 1e-3f );
	EXPECT_NEAR( camera->GetPitch(), 0.0f, 1e-3f );
}

TEST( CameraComponentTests, SetRotationAnglesUpdatesTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();
	camera->SetRotationAngles( 45.0f, -20.0f );

	const Quat rotation = entity->GetRotation();

	EXPECT_TRUE( IsClose( rotation.Rotate( Vec3::V_Front ), FrontFromAngles( 45.0f, -20.0f ), 1e-4f ) );
	EXPECT_NEAR( rotation.Rotate( Vec3::V_Right ).y, 0.0f, 1e-4f );
}

TEST( CameraComponentTests, PitchIsClampedToAvoidLookingStraightUp )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	camera->SetRotationAngles( 0.0f, 120.0f );
	EXPECT_FLOAT_EQ( camera->GetPitch(), 89.0f );

	camera->SetRotationAngles( 0.0f, -120.0f );
	EXPECT_FLOAT_EQ( camera->GetPitch(), -89.0f );
}

TEST( CameraComponentTests, AnglesRoundTripThroughTheRotationWithoutMoving )
{
	const float angles[][2] = { { 90.0f, 0.0f }, { 60.0f, 0.0f }, { 45.0f, -20.0f }, { -135.0f, 40.0f }, { 170.0f, 80.0f } };

	for( const auto& pair : angles )
	{
		Level   level;
		Entity* source = level.CreateEntity( "Source" );

		shared<CameraComponent> sourceCamera = source->AddComponent<CameraComponent>();
		sourceCamera->SetRotationAngles( pair[0], pair[1] );

		Entity* restored = level.CreateEntity( "Restored" );
		restored->SetRotation( source->GetRotation() );

		shared<CameraComponent> restoredCamera = restored->AddComponent<CameraComponent>();
		EXPECT_NEAR( restoredCamera->GetYaw(), pair[0], 1e-2f );
		EXPECT_NEAR( restoredCamera->GetPitch(), pair[1], 1e-2f );

		// Note: Applying the restored angles must not make the camera jump
		const Quat before = restored->GetRotation();
		restoredCamera->SetRotationAngles( restoredCamera->GetYaw(), restoredCamera->GetPitch() );
		EXPECT_NEAR( fabsf( Dot( before, restored->GetRotation() ) ), 1.0f, 1e-5f );
	}
}

TEST( CameraComponentTests, YawAndPitchAreNotSaved )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->AddComponent<CameraComponent>();

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_Yaw" ) );
	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_Pitch" ) );
}

TEST( CameraComponentTests, SavedLevelRestoresWhereTheCameraWasLooking )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "camera.lvl" );
	{
		Level   saved;
		Entity* entity = saved.CreateEntity( "Camera" );

		shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();
		camera->SetRotationAngles( 45.0f, -20.0f );
		ASSERT_TRUE( saved.SaveAs( path ) );
	}

	Level loaded( path );
	ASSERT_TRUE( loaded.Load() );

	Entity* entity = loaded.FindEntity( 1 );
	ASSERT_NE( entity, nullptr );
	EXPECT_TRUE( IsClose( entity->GetRotation().Rotate( Vec3::V_Front ), FrontFromAngles( 45.0f, -20.0f ), 1e-4f ) );
	EXPECT_NEAR( entity->GetComponent<CameraComponent>()->GetYaw(), 45.0f, 1e-2f );
	EXPECT_NEAR( entity->GetComponent<CameraComponent>()->GetPitch(), -20.0f, 1e-2f );
}

TEST( CameraComponentTests, YawAndPitchInOldLevelFilesAreIgnored )
{
	ScopedTempDir directory;

	Level level( directory.Write( "old.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Camera",
			"m_SerializedComponents": [ { "m_TypeName": "CameraComponent", "m_Data": { "m_Yaw": 10.0, "m_Pitch": 5.0 } } ]
		} ]
	})" ) );

	ASSERT_TRUE( level.Load() );

	EXPECT_NEAR( level.FindEntity( 1 )->GetComponent<CameraComponent>()->GetYaw(), -90.0f, 1e-3f );
}

TEST( CameraComponentTests, SpeedsAreSharedAndNotSaved )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->AddComponent<CameraComponent>();

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_LinearSpeed" ) );
	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_AngularSpeed" ) );

	CameraComponent::SetLinearSpeed( 0.5f );
	CameraComponent::SetAngularSpeed( 1.5f );
	EXPECT_EQ( CameraComponent::GetLinearSpeed(), 0.5f );
	EXPECT_EQ( CameraComponent::GetAngularSpeed(), 1.5f );

	// Note: Restored so other tests see the defaults
	CameraComponent::SetLinearSpeed( CameraComponent::kDefaultLinearSpeed );
	CameraComponent::SetAngularSpeed( CameraComponent::kDefaultAngularSpeed );
}

TEST( CameraComponentTests, SpeedsInOldLevelFilesAreIgnored )
{
	ScopedTempDir directory;

	Level level( directory.Write( "old.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Camera",
			"m_SerializedComponents": [ { "m_TypeName": "CameraComponent", "m_Data": { "m_LinearSpeed": 3.0, "m_AngularSpeed": 4.0 } } ]
		} ]
	})" ) );

	ASSERT_TRUE( level.Load() );

	EXPECT_EQ( CameraComponent::GetLinearSpeed(), CameraComponent::kDefaultLinearSpeed );
	EXPECT_EQ( CameraComponent::GetAngularSpeed(), CameraComponent::kDefaultAngularSpeed );
}

TEST( CameraComponentTests, ViewMatrixMovesTheCameraToTheOrigin )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->SetPosition( Vec3( 1.0f, 2.0f, -3.0f ) );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	const Matrix44 view = camera->GetViewMatrix();
	const Vec4     position( entity->GetPosition(), 1.0f );

	EXPECT_NEAR( Dot( view.GetRow( 0 ), position ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( view.GetRow( 1 ), position ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( view.GetRow( 2 ), position ), 0.0f, 1e-5f );
}

TEST( CameraComponentTests, ProjectionKeepsHorizontalFovAcrossAspectRatios )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	const Matrix44 wide   = camera->GetProjectionMatrix( rect{ 1600, 900 } );
	const Matrix44 square = camera->GetProjectionMatrix( rect{ 900, 900 } );

	EXPECT_NEAR( wide.m00, 1.0f, 1e-5f );
	EXPECT_NEAR( square.m00, 1.0f, 1e-5f );
	EXPECT_NEAR( wide.m11, 1600.0f / 900.0f, 1e-5f );
	EXPECT_NEAR( square.m11, 1.0f, 1e-5f );
}

TEST( ModelComponentTests, ModelMatrixFollowsTheEntityTransform )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetModel( "missing.obj" );

	entity->SetPosition( Vec3( 1.0f, 2.0f, 3.0f ) );
	entity->SetScale( Vec3( 2.0f, 2.0f, 2.0f ) );

	const Matrix44 matrix = model->GetModelMatrix();

	EXPECT_FLOAT_EQ( matrix.m03, 1.0f );
	EXPECT_FLOAT_EQ( matrix.m13, 2.0f );
	EXPECT_FLOAT_EQ( matrix.m23, 3.0f );
	EXPECT_FLOAT_EQ( matrix.m00, 2.0f );
}

TEST( ModelComponentTests, DrawWithoutAResourceDoesNothing )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetModel( "missing.obj" );

	const Renderable& renderable = *model;
	renderable.Draw( DrawContext() );
}

TEST( ModelComponentTests, ModelIsSavedAsAResourceName )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );
	entity->AddComponent<ModelComponent>()->SetModel( "cube.obj" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_EQ( json[0]["m_TypeName"].asString(), "ModelComponent" );
	EXPECT_EQ( json[0]["m_Data"]["m_Model"].asString(), "cube.obj" );
}

TEST( ModelComponentTests, EmptyModelIsValidAndLogsNothing )
{
	Level      level;
	Entity*    entity = level.CreateEntity( "Model" );
	LogCapture log;

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();

	EXPECT_FALSE( log.Contains( "error" ) );

	const Renderable& renderable = *model;
	renderable.Draw( DrawContext() );
}

TEST( ModelComponentTests, EmptyResourcesStayValidAsPropertiesChange )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();

	LogCapture log;
	model->SetModel( "" );
	model->SetMaterialOverride( "" );
	model->OnPropertyChanged( "m_Model" );
	model->OnPropertyChanged( "m_MaterialOverride" );

	EXPECT_FALSE( log.Contains( "error" ) );
	EXPECT_FALSE( log.Contains( "warning" ) );

	const Renderable& renderable = *model;
	renderable.Draw( DrawContext() );
}

TEST( ModelComponentTests, AnEmptyModelKeepsItsOverride )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetMaterialOverride( "error" );
	model->OnPropertyChanged( "m_Model" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );
	EXPECT_EQ( json[0]["m_Data"]["m_Model"].asString(), "" );
	EXPECT_EQ( json[0]["m_Data"]["m_MaterialOverride"].asString(), "error" );
}

TEST( ModelComponentTests, MaterialOverrideIsSavedAsAResourceName )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );
	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetModel( "cube" );
	model->SetMaterialOverride( "error" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_EQ( json[0]["m_Data"]["m_Model"].asString(), "cube" );
	EXPECT_EQ( json[0]["m_Data"]["m_MaterialOverride"].asString(), "error" );
}

TEST( ModelComponentTests, ClearingTheMaterialOverrideSavesItEmpty )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetMaterialOverride( "error" );
	model->SetMaterialOverride( "" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );
	EXPECT_EQ( json[0]["m_Data"]["m_MaterialOverride"].asString(), "" );
}

TEST( ModelComponentTests, ChangingTheModelClearsTheMaterialOverride )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetModel( "first" );
	model->SetMaterialOverride( "error" );
	model->SetModel( "second" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );
	EXPECT_EQ( json[0]["m_Data"]["m_Model"].asString(), "second" );
	EXPECT_EQ( json[0]["m_Data"]["m_MaterialOverride"].asString(), "" );
}

TEST( ModelComponentTests, SettingTheSameModelKeepsTheMaterialOverride )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetModel( "first" );
	model->SetMaterialOverride( "error" );
	model->SetModel( "first" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );
	EXPECT_EQ( json[0]["m_Data"]["m_MaterialOverride"].asString(), "error" );
}

TEST( ModelComponentTests, AnOverrideIsAcceptedWhenTheModelDoesNotUseIt )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	// Note: Without a session the model doesn't load, so it uses no library and nothing is refused
	EXPECT_TRUE( entity->AddComponent<ModelComponent>()->SetMaterialOverride( "error" ) );
}

TEST( ModelComponentTests, AnUnloadedModelUsesNoMaterialLibrary )
{
	const Model model( "models/sponza.obj" );

	EXPECT_FALSE( model.UsesMaterialLibrary( "sponza" ) );
	EXPECT_FALSE( model.UsesMaterialLibrary( "" ) );
}

TEST( ModelComponentTests, SetMaterialOverrideWithoutAnEntityLoadsNothing )
{
	LogCapture log;

	ModelComponent model;
	model.SetMaterialOverride( "test" );

	EXPECT_FALSE( log.Contains( "material test is not available" ) );
}

TEST( ModelComponentTests, MaterialOverrideIsAResourceProperty )
{
	ModelComponent model;

	const rtti::PropertyInfo* property = model.GetTypeInfo().FindProperty( "m_MaterialOverride" );

	ASSERT_NE( property, nullptr );
	EXPECT_EQ( property->ResourceTypeId, GetTypeId<Material>() );
}

TEST( ModelComponentTests, MissingMaterialIsReported )
{
	Level      level;
	Entity*    entity = level.CreateEntity( "Model" );
	LogCapture log;

	entity->AddComponent<ModelComponent>()->SetMaterialOverride( "test" );

	EXPECT_TRUE( log.Contains( "warning: ModelComponent: material test is not available, using error" ) );
}

TEST( ModelComponentTests, LevelsSavedBeforeMaterialsStillLoad )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	Json::Value oldComponent( Json::objectValue );
	oldComponent["m_TypeName"]        = "ModelComponent";
	oldComponent["m_Data"]["m_Model"] = "";

	Json::Value components( Json::arrayValue );
	components.append( oldComponent );

	EXPECT_TRUE( entity->DeserializeComponents( components ) );
	EXPECT_TRUE( entity->HasComponent( "ModelComponent" ) );
}

TEST( ModelComponentTests, MissingModelIsReported )
{
	Level      level;
	Entity*    entity = level.CreateEntity( "Model" );
	LogCapture log;

	entity->AddComponent<ModelComponent>()->SetModel( "missing" );

	EXPECT_TRUE( log.Contains( "error: ModelComponent: failed to load missing" ) );
}

TEST( ModelComponentTests, SetModelWithoutAnEntityLoadsNothing )
{
	LogCapture log;

	ModelComponent model;
	model.SetModel( "missing" );

	EXPECT_FALSE( log.Contains( "failed to load missing" ) );
}

TEST( ModelComponentTests, SetModelSavesTheNewName )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();
	model->SetModel( "first" );
	model->SetModel( "second" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );
	EXPECT_EQ( json[0]["m_Data"]["m_Model"].asString(), "second" );
}

TEST( ModelComponentTests, ChangingThePropertyResolvesTheNewModel )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>();

	const rtti::PropertyInfo* property = model->GetTypeInfo().FindProperty( "m_Model" );
	ASSERT_NE( property, nullptr );
	ASSERT_NE( property->ResourceTypeId, 0u );

	void* valuePtr = reinterpret_cast<u8*>( static_cast<Component*>( model.get() ) ) + property->Offset;
	ASSERT_TRUE( property->Deserialize( valuePtr, Json::Value( "missing" ) ) );

	LogCapture log;
	model->OnPropertyChanged( property->Name );

	EXPECT_TRUE( log.Contains( "error: ModelComponent: failed to load missing" ) );
}

TEST( CameraComponentTests, EditingTheTransformUpdatesYawAndPitch )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	entity->SetRotation( LookAtQuat( FrontFromAngles( 60.0f, -20.0f ), Vec3::V_Up ) );

	EXPECT_NEAR( camera->GetYaw(), 60.0f, 1e-2f );
	EXPECT_NEAR( camera->GetPitch(), -20.0f, 1e-2f );
}

TEST( CameraComponentTests, ClonedCameraKeepsItsOrientation )
{
	Level   level;
	Entity* source = level.CreateEntity( "Camera" );

	shared<CameraComponent> camera = source->AddComponent<CameraComponent>();
	camera->SetRotationAngles( 45.0f, -20.0f );

	Entity* clone = level.CloneEntity( source->GetId() );
	ASSERT_NE( clone, nullptr );

	const CameraComponent* clonedCamera = clone->GetComponent<CameraComponent>();
	ASSERT_NE( clonedCamera, nullptr );
	EXPECT_NEAR( clonedCamera->GetYaw(), 45.0f, 1e-2f );
	EXPECT_NEAR( clonedCamera->GetPitch(), -20.0f, 1e-2f );
}
