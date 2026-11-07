#include "testHelpers.h"

#include <components/cameraComponent.h>
#include <components/modelComponent.h>
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
	entity->GetTransformRef().rotation = rotation;

	entity->AddComponent<CameraComponent>();

	EXPECT_EQ( entity->GetTransform().rotation.x, rotation.x );
	EXPECT_EQ( entity->GetTransform().rotation.y, rotation.y );
	EXPECT_EQ( entity->GetTransform().rotation.z, rotation.z );
	EXPECT_EQ( entity->GetTransform().rotation.w, rotation.w );
}

TEST( CameraComponentTests, AnglesAreDerivedFromTheEntityRotation )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );

	entity->GetTransformRef().rotation = LookAtQuat( FrontFromAngles( 60.0f, -20.0f ), Vec3::V_Up );

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

	const Quat rotation = entity->GetTransform().rotation;

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

		Entity* restored                     = level.CreateEntity( "Restored" );
		restored->GetTransformRef().rotation = source->GetTransform().rotation;

		shared<CameraComponent> restoredCamera = restored->AddComponent<CameraComponent>();
		EXPECT_NEAR( restoredCamera->GetYaw(), pair[0], 1e-2f );
		EXPECT_NEAR( restoredCamera->GetPitch(), pair[1], 1e-2f );

		// Note: Applying the restored angles must not make the camera jump
		const Quat before = restored->GetTransform().rotation;
		restoredCamera->SetRotationAngles( restoredCamera->GetYaw(), restoredCamera->GetPitch() );
		EXPECT_NEAR( fabsf( Dot( before, restored->GetTransform().rotation ) ), 1.0f, 1e-5f );
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
	ScopedTempDir directory;
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
	EXPECT_TRUE( IsClose( entity->GetTransform().rotation.Rotate( Vec3::V_Front ), FrontFromAngles( 45.0f, -20.0f ), 1e-4f ) );
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

TEST( CameraComponentTests, ViewMatrixMovesTheCameraToTheOrigin )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->GetTransformRef().position = Vec3( 1.0f, 2.0f, -3.0f );

	shared<CameraComponent> camera = entity->AddComponent<CameraComponent>();

	const Matrix44 view = camera->GetViewMatrix();
	const Vec4     position( entity->GetTransform().position, 1.0f );

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

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>( "missing.obj" );

	entity->GetTransformRef().position = Vec3( 1.0f, 2.0f, 3.0f );
	entity->GetTransformRef().scale    = Vec3( 2.0f, 2.0f, 2.0f );

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

	shared<ModelComponent> model = entity->AddComponent<ModelComponent>( "missing.obj" );

	const Renderable& renderable = *model;
	renderable.Draw();
}

TEST( ModelComponentTests, ModelNameIsSavedWithTheEntity )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Model" );
	entity->AddComponent<ModelComponent>( "cube.obj" );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	EXPECT_EQ( json[0]["m_TypeName"].asString(), "ModelComponent" );
	EXPECT_EQ( json[0]["m_Data"]["modelName"].asString(), "cube.obj" );
}
