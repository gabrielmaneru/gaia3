#include <gtest/gtest.h>

#include <components/cameraComponent.h>
#include <components/modelComponent.h>
#include <math/mathHelpers.h>
#include <math/vectorTransforms.h>
#include <scene/level.h>

TEST( CameraComponentTests, DefaultCameraLooksAlongPositiveZ )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Camera" );
	entity->AddComponent<CameraComponent>();

	const Vec3 forward = entity->GetTransform().rotation.Rotate( Vec3::V_Front );

	EXPECT_TRUE( IsClose( forward, Vec3( 0.0f, 0.0f, 1.0f ), 1e-5f ) );
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
