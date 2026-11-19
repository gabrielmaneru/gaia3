#include "testHelpers.h"

#include <components/cameraComponent.h>
#include <components/lightComponent.h>
#include <components/modelComponent.h>
#include <core/app.h>
#include <gfx/material.h>
#include <gfx/model.h>
#include <math/mathHelpers.h>
#include <math/quaternionTransforms.h>
#include <math/vectorTransforms.h>
#include <scene/level.h>

#include <algorithm>
#include <math.h>
#include <type_traits>

namespace
{
Vec3 FrontFromAngles( float yaw, float pitch )
{
	const float cosPitch = cosf( ToRadians( pitch ) );
	return Vec3( cosf( ToRadians( yaw ) ) * cosPitch, sinf( ToRadians( pitch ) ), sinf( ToRadians( yaw ) ) * cosPitch );
}
} // namespace

TEST( FreeCameraComponentTests, AddingTheComponentKeepsTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	const Quat rotation = LookAtQuat( Vec3( 0.3f, 0.4f, 0.8f ).Normalized(), Vec3::V_Up );
	entity->SetRotation( rotation );

	entity->AddComponent<FreeCameraComponent>();

	EXPECT_EQ( entity->GetRotation().x, rotation.x );
	EXPECT_EQ( entity->GetRotation().y, rotation.y );
	EXPECT_EQ( entity->GetRotation().z, rotation.z );
	EXPECT_EQ( entity->GetRotation().w, rotation.w );
}

TEST( FreeCameraComponentTests, AnglesAreDerivedFromTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	entity->SetRotation( LookAtQuat( FrontFromAngles( 60.0f, -20.0f ), Vec3::V_Up ) );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();

	EXPECT_NEAR( camera->GetYaw(), 60.0f, 1e-2f );
	EXPECT_NEAR( camera->GetPitch(), -20.0f, 1e-2f );
}

TEST( FreeCameraComponentTests, IdentityRotationLooksAlongTheFrontAxis )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();

	EXPECT_NEAR( camera->GetYaw(), -90.0f, 1e-3f );
	EXPECT_NEAR( camera->GetPitch(), 0.0f, 1e-3f );
}

TEST( FreeCameraComponentTests, SetRotationAnglesUpdatesTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();
	camera->SetRotationAngles( 45.0f, -20.0f );

	const Quat rotation = entity->GetRotation();

	EXPECT_TRUE( IsClose( rotation.Rotate( Vec3::V_Front ), FrontFromAngles( 45.0f, -20.0f ), 1e-4f ) );
	EXPECT_NEAR( rotation.Rotate( Vec3::V_Right ).y, 0.0f, 1e-4f );
}

TEST( FreeCameraComponentTests, PitchIsClampedToAvoidLookingStraightUp )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();

	camera->SetRotationAngles( 0.0f, 120.0f );
	EXPECT_FLOAT_EQ( camera->GetPitch(), 89.0f );

	camera->SetRotationAngles( 0.0f, -120.0f );
	EXPECT_FLOAT_EQ( camera->GetPitch(), -89.0f );
}

TEST( FreeCameraComponentTests, AnglesRoundTripThroughTheRotationWithoutMoving )
{
	const float angles[][2] = { { 90.0f, 0.0f }, { 60.0f, 0.0f }, { 45.0f, -20.0f }, { -135.0f, 40.0f }, { 170.0f, 80.0f } };

	for( const auto& pair : angles )
	{
		Level   level;
		Entity* source = level.CreateEntity( "Source" );

		shared<FreeCameraComponent> sourceCamera = source->AddComponent<FreeCameraComponent>();
		sourceCamera->SetRotationAngles( pair[0], pair[1] );

		Entity* restored = level.CreateEntity( "Restored" );
		restored->SetRotation( source->GetRotation() );

		shared<FreeCameraComponent> restoredCamera = restored->AddComponent<FreeCameraComponent>();
		EXPECT_NEAR( restoredCamera->GetYaw(), pair[0], 1e-2f );
		EXPECT_NEAR( restoredCamera->GetPitch(), pair[1], 1e-2f );

		// Note: Applying the restored angles must not make the camera jump
		const Quat before = restored->GetRotation();
		restoredCamera->SetRotationAngles( restoredCamera->GetYaw(), restoredCamera->GetPitch() );
		EXPECT_NEAR( fabsf( Dot( before, restored->GetRotation() ) ), 1.0f, 1e-5f );
	}
}

TEST( FreeCameraComponentTests, YawAndPitchAreNotSaved )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->AddComponent<FreeCameraComponent>();

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_Yaw" ) );
	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_Pitch" ) );
}

TEST( FreeCameraComponentTests, SavedLevelRestoresWhereTheCameraWasLooking )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "camera.lvl" );
	{
		Level   saved;
		Entity* entity = saved.CreateEntity( "Camera" );

		shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();
		camera->SetRotationAngles( 45.0f, -20.0f );
		ASSERT_TRUE( saved.SaveAs( path ) );
	}

	Level loaded( path );
	ASSERT_TRUE( loaded.Load() );

	Entity* entity = loaded.FindEntity( 1 );
	ASSERT_NE( entity, nullptr );
	EXPECT_TRUE( IsClose( entity->GetRotation().Rotate( Vec3::V_Front ), FrontFromAngles( 45.0f, -20.0f ), 1e-4f ) );
	EXPECT_NEAR( entity->GetComponent<FreeCameraComponent>()->GetYaw(), 45.0f, 1e-2f );
	EXPECT_NEAR( entity->GetComponent<FreeCameraComponent>()->GetPitch(), -20.0f, 1e-2f );
}

TEST( FreeCameraComponentTests, YawAndPitchInOldLevelFilesAreIgnored )
{
	ScopedTempDir directory;

	Level level( directory.Write( "old.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Camera",
			"m_SerializedComponents": [ { "m_TypeName": "FreeCameraComponent", "m_Data": { "m_Yaw": 10.0, "m_Pitch": 5.0 } } ]
		} ]
	})" ) );

	ASSERT_TRUE( level.Load() );

	EXPECT_NEAR( level.FindEntity( 1 )->GetComponent<FreeCameraComponent>()->GetYaw(), -90.0f, 1e-3f );
}

TEST( FreeCameraComponentTests, SpeedsAreSharedAndNotSaved )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->AddComponent<FreeCameraComponent>();

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_LinearSpeed" ) );
	EXPECT_FALSE( json[0]["m_Data"].isMember( "m_AngularSpeed" ) );

	FreeCameraComponent::SetLinearSpeed( 0.5f );
	FreeCameraComponent::SetAngularSpeed( 1.5f );
	EXPECT_EQ( FreeCameraComponent::GetLinearSpeed(), 0.5f );
	EXPECT_EQ( FreeCameraComponent::GetAngularSpeed(), 1.5f );

	// Note: Restored so other tests see the defaults
	FreeCameraComponent::SetLinearSpeed( FreeCameraComponent::kDefaultLinearSpeed );
	FreeCameraComponent::SetAngularSpeed( FreeCameraComponent::kDefaultAngularSpeed );
}

TEST( FreeCameraComponentTests, SpeedsInOldLevelFilesAreIgnored )
{
	ScopedTempDir directory;

	Level level( directory.Write( "old.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Camera",
			"m_SerializedComponents": [ { "m_TypeName": "FreeCameraComponent", "m_Data": { "m_LinearSpeed": 3.0, "m_AngularSpeed": 4.0 } } ]
		} ]
	})" ) );

	ASSERT_TRUE( level.Load() );

	EXPECT_EQ( FreeCameraComponent::GetLinearSpeed(), FreeCameraComponent::kDefaultLinearSpeed );
	EXPECT_EQ( FreeCameraComponent::GetAngularSpeed(), FreeCameraComponent::kDefaultAngularSpeed );
}

TEST( FreeCameraComponentTests, ViewMatrixMovesTheCameraToTheOrigin )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->SetPosition( Vec3( 1.0f, 2.0f, -3.0f ) );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();

	const Matrix44 view = camera->GetViewMatrix();
	const Vec4     position( entity->GetPosition(), 1.0f );

	EXPECT_NEAR( Dot( view.GetRow( 0 ), position ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( view.GetRow( 1 ), position ), 0.0f, 1e-5f );
	EXPECT_NEAR( Dot( view.GetRow( 2 ), position ), 0.0f, 1e-5f );
}

TEST( FreeCameraComponentTests, ProjectionKeepsHorizontalFovAcrossAspectRatios )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();

	const Matrix44 wide   = camera->GetProjectionMatrix( rect{ 1600, 900 } );
	const Matrix44 square = camera->GetProjectionMatrix( rect{ 900, 900 } );

	EXPECT_NEAR( wide.m00, 1.0f, 1e-5f );
	EXPECT_NEAR( square.m00, 1.0f, 1e-5f );
	EXPECT_NEAR( wide.m11, 1600.0f / 900.0f, 1e-5f );
	EXPECT_NEAR( square.m11, 1.0f, 1e-5f );
}

// Note: The bases can't be created on their own, only the component types built on them, checked while compiling
static_assert( std::is_abstract_v<Component> );
static_assert( std::is_abstract_v<CameraComponent> );
static_assert( std::is_abstract_v<LightComponent> );
static_assert( !std::is_default_constructible_v<Component> );
static_assert( !std::is_default_constructible_v<CameraComponent> );
static_assert( !std::is_default_constructible_v<LightComponent> );

static_assert( std::is_default_constructible_v<FreeCameraComponent> );
static_assert( std::is_default_constructible_v<OrbitCameraComponent> );
static_assert( std::is_default_constructible_v<DirectionalLightComponent> );
static_assert( std::is_default_constructible_v<PointLightComponent> );
static_assert( std::is_default_constructible_v<ModelComponent> );

TEST( CameraComponentTests, BothCameraTypesAreCamerasAndRegisteredComponents )
{
	const std::vector<std::string> names = ComponentFactory::GetRegisteredTypeNames();
	EXPECT_NE( std::find( names.begin(), names.end(), "FreeCameraComponent" ), names.end() );
	EXPECT_NE( std::find( names.begin(), names.end(), "OrbitCameraComponent" ), names.end() );

	// Note: The shared base isn't a component of its own
	EXPECT_EQ( std::find( names.begin(), names.end(), "CameraComponent" ), names.end() );

	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> freeCamera = entity->AddComponent<FreeCameraComponent>();
	const CameraComponent&      camera     = *freeCamera;
	EXPECT_FLOAT_EQ( camera.GetHorizontalFOV(), 90.0f );
	EXPECT_EQ( App::GetRenderer().GetCamera(), static_cast<const RenderCamera*>( freeCamera.get() ) );

	entity->RemoveComponent( freeCamera.get() );
	EXPECT_EQ( App::GetRenderer().GetCamera(), nullptr );
}

TEST( OrbitCameraComponentTests, LooksAtItsTargetFromItsDistance )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<OrbitCameraComponent> orbit = entity->AddComponent<OrbitCameraComponent>();
	orbit->SetOrbit( Vec3( 1.0f, 2.0f, 3.0f ), 10.0f, 0.0f, 0.0f );

	// Note: Zero yaw and pitch looks along -Z, so the camera sits 10 along +Z from the target
	EXPECT_TRUE( IsClose( entity->GetPosition(), Vec3( 1.0f, 2.0f, 13.0f ), 1e-4f ) );
	EXPECT_TRUE( IsClose( entity->GetRotation().Rotate( Vec3::V_Front ), Vec3( 0.0f, 0.0f, -1.0f ), 1e-4f ) );

	// Note: A quarter turn of yaw looks along -X instead
	orbit->SetOrbit( Vec3::V_Zero, 4.0f, 90.0f, 0.0f );
	EXPECT_TRUE( IsClose( entity->GetPosition(), Vec3( 4.0f, 0.0f, 0.0f ), 1e-4f ) );
}

TEST( OrbitCameraComponentTests, PitchStaysShortOfStraightUpOrDown )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<OrbitCameraComponent> orbit = entity->AddComponent<OrbitCameraComponent>();
	orbit->SetOrbit( Vec3::V_Zero, 1.0f, 0.0f, -120.0f );

	EXPECT_FLOAT_EQ( orbit->GetPitch(), -89.0f );
}

TEST( OrbitCameraComponentTests, FramingSeesTheWholeBox )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<OrbitCameraComponent> orbit = entity->AddComponent<OrbitCameraComponent>();
	orbit->SetHorizontalFOV( 70.0f );

	AABB box;
	box.Add( Vec3( -1.0f, 0.0f, -2.0f ) );
	box.Add( Vec3( 3.0f, 2.0f, 2.0f ) );
	orbit->Frame( box );

	EXPECT_TRUE( IsClose( orbit->GetTarget(), Vec3( 1.0f, 1.0f, 0.0f ), 1e-5f ) );

	// Note: The camera stays outside the box's sphere, and every corner lands inside the view
	EXPECT_GT( orbit->GetDistance(), box.GetRadius() );
	const Matrix44 viewProjection = orbit->GetProjectionMatrix( rect{ 1600, 1000 } ) * orbit->GetViewMatrix();
	for( const Vec3& corner : { box.min, box.max, Vec3( box.min.x, box.max.y, box.min.z ), Vec3( box.max.x, box.min.y, box.max.z ) } )
	{
		const Vec4 clip = viewProjection * Vec4( corner, 1.0f );
		ASSERT_GT( clip.w, 0.0f );
		EXPECT_LE( fabsf( clip.x / clip.w ), 1.0f );
		EXPECT_LE( fabsf( clip.y / clip.w ), 1.0f );
	}

	// Note: An empty box leaves the camera where it is
	const float distance = orbit->GetDistance();
	orbit->Frame( AABB() );
	EXPECT_FLOAT_EQ( orbit->GetDistance(), distance );
}

TEST( OrbitCameraComponentTests, ItsPlanesFollowItsDistance )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<OrbitCameraComponent> orbit = entity->AddComponent<OrbitCameraComponent>();
	orbit->SetOrbit( Vec3::V_Zero, 1000.0f, 0.0f, 0.0f );

	// Note: A point 2000 away, past the saved far plane of 100, still projects in front of the far plane
	const Matrix44 viewProjection = orbit->GetProjectionMatrix( rect{ 100, 100 } ) * orbit->GetViewMatrix();
	const Vec4     clip           = viewProjection * Vec4( 0.0f, 0.0f, -1000.0f, 1.0f );
	EXPECT_LT( clip.z / clip.w, 1.0f );
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

TEST( FreeCameraComponentTests, EditingTheTransformUpdatesYawAndPitch )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> camera = entity->AddComponent<FreeCameraComponent>();

	entity->SetRotation( LookAtQuat( FrontFromAngles( 60.0f, -20.0f ), Vec3::V_Up ) );

	EXPECT_NEAR( camera->GetYaw(), 60.0f, 1e-2f );
	EXPECT_NEAR( camera->GetPitch(), -20.0f, 1e-2f );
}

TEST( FreeCameraComponentTests, ClonedCameraKeepsItsOrientation )
{
	Level   level;
	Entity* source = level.CreateEntity( "Camera" );

	shared<FreeCameraComponent> camera = source->AddComponent<FreeCameraComponent>();
	camera->SetRotationAngles( 45.0f, -20.0f );

	Entity* clone = level.CloneEntity( source->GetId() );
	ASSERT_NE( clone, nullptr );

	const FreeCameraComponent* clonedCamera = clone->GetComponent<FreeCameraComponent>();
	ASSERT_NE( clonedCamera, nullptr );
	EXPECT_NEAR( clonedCamera->GetYaw(), 45.0f, 1e-2f );
	EXPECT_NEAR( clonedCamera->GetPitch(), -20.0f, 1e-2f );
}
