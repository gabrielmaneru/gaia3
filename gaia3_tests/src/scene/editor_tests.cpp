#include "testHelpers.h"

#include "testComponents.h"

#include <core/app.h>
#include <math/quaternionTransforms.h>

namespace
{
void ExpectSameRotation( const Quat& actual, const Quat& expected )
{
	EXPECT_FLOAT_EQ( actual.x, expected.x );
	EXPECT_FLOAT_EQ( actual.y, expected.y );
	EXPECT_FLOAT_EQ( actual.z, expected.z );
	EXPECT_FLOAT_EQ( actual.w, expected.w );
}

class EditorTests : public testing::Test
{
protected:
	void SetUp() override { ASSERT_TRUE( App::GetSession().Init( "", "", "" ) ); }
	void TearDown() override { App::GetSession().Shutdown(); }

	Level* GetSessionLevel() { return App::GetSession().GetCurrentLevel(); }
};

} // namespace

TEST( EditorDefaultsTests, EditorIsDisabledByDefault )
{
	const AppConfig config;

	EXPECT_FALSE( config.editor.enabled );
	EXPECT_FALSE( config.editor.docked );
}

TEST( EditorDefaultsTests, WindowScaleIsOneWithoutAWindow )
{
	const Window window;

	EXPECT_FLOAT_EQ( window.GetContentScale(), 1.0f );
}

TEST( EditorDefaultsTests, AppRunsUntilClosedByDefault )
{
	const AppConfig config;

	EXPECT_EQ( config.maxFrames, 0u );
}

TEST( EditorDefaultsTests, FloatingEditorLeavesTheGameInTheWindow )
{
	const Editor editor( EditorConfig{ true, false } );

	EXPECT_FALSE( editor.DisplaysRenderTarget() );
}

TEST( EditorDefaultsTests, DockedEditorDisplaysTheGameInItsViewport )
{
	const Editor editor( EditorConfig{ true, true } );

	EXPECT_TRUE( editor.DisplaysRenderTarget() );
}

TEST( EditorDefaultsTests, GizmoStartsInSelectionMode )
{
	const EditorConfig config;
	const Editor       editor( config );

	EXPECT_EQ( config.gizmoMode, GizmoMode::Selection );
	EXPECT_EQ( editor.GetGizmoMode(), GizmoMode::Selection );
}

TEST( EditorDefaultsTests, GizmoStartsInTheConfiguredMode )
{
	EditorConfig config;
	config.gizmoMode = GizmoMode::Translate;

	const Editor editor( config );

	EXPECT_EQ( editor.GetGizmoMode(), GizmoMode::Translate );
}

TEST( EditorDefaultsTests, GizmoModeCanChange )
{
	Editor editor;

	editor.SetGizmoMode( GizmoMode::Scale );

	EXPECT_EQ( editor.GetGizmoMode(), GizmoMode::Scale );
}

TEST( GizmoMatrixTests, SelectionChangesNothing )
{
	const Transform transform( Vec3( 1.0f, 2.0f, 3.0f ), QuatFromEuler( Vec3( 10.0f, 20.0f, 30.0f ) ), Vec3( 2.0f, 3.0f, 4.0f ) );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Selection, transform, Transform().ToMatrix44() );

	EXPECT_EQ( result.position, transform.position );
	EXPECT_EQ( result.scale, transform.scale );
	ExpectSameRotation( result.rotation, transform.rotation );
}

TEST( GizmoMatrixTests, TranslateOnlyChangesThePosition )
{
	const Transform transform( Vec3( 1.0f, 2.0f, 3.0f ), QuatFromEuler( Vec3( 10.0f, 20.0f, 30.0f ) ), Vec3( 2.0f, 3.0f, 4.0f ) );

	Transform moved = transform;
	moved.position  = Vec3( -4.0f, 5.0f, 6.0f );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Translate, transform, moved.ToMatrix44() );

	EXPECT_EQ( result.position, moved.position );
	ExpectSameRotation( result.rotation, transform.rotation );
	EXPECT_EQ( result.scale, transform.scale );
}

TEST( GizmoMatrixTests, RotateOnlyChangesTheRotation )
{
	const Transform transform( Vec3( 1.0f, 2.0f, 3.0f ), Quat(), Vec3( 2.0f, 3.0f, 4.0f ) );

	Transform rotated = transform;
	rotated.rotation  = QuatFromEuler( Vec3( 10.0f, 20.0f, 30.0f ) );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Rotate, transform, rotated.ToMatrix44() );

	const Vec3 expected = rotated.rotation.Rotate( Vec3::V_Right );
	const Vec3 actual   = result.rotation.Rotate( Vec3::V_Right );
	EXPECT_NEAR( actual.x, expected.x, 0.001f );
	EXPECT_NEAR( actual.y, expected.y, 0.001f );
	EXPECT_NEAR( actual.z, expected.z, 0.001f );
	EXPECT_EQ( result.position, transform.position );
	EXPECT_EQ( result.scale, transform.scale );
}

TEST( GizmoMatrixTests, RotateKeepsAMirroredEntityUsable )
{
	const Transform transform( Vec3(), Quat(), Vec3( -2.0f, 1.0f, 1.0f ) );

	Transform rotated = transform;
	rotated.rotation  = QuatFromEuler( Vec3( 0.0f, 90.0f, 0.0f ) );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Rotate, transform, rotated.ToMatrix44() );

	const Vec3 expected = rotated.rotation.Rotate( Vec3::V_Front );
	const Vec3 actual   = result.rotation.Rotate( Vec3::V_Front );
	EXPECT_NEAR( actual.x, expected.x, 0.001f );
	EXPECT_NEAR( actual.y, expected.y, 0.001f );
	EXPECT_NEAR( actual.z, expected.z, 0.001f );
}

TEST( GizmoMatrixTests, RotateKeepsTheRotationOfAZeroScale )
{
	const Quat      rotation = QuatFromEuler( Vec3( 10.0f, 20.0f, 30.0f ) );
	const Transform transform( Vec3(), rotation, Vec3( 0.0f, 1.0f, 1.0f ) );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Rotate, transform, Transform().ToMatrix44() );

	ExpectSameRotation( result.rotation, rotation );
}

TEST( GizmoMatrixTests, ScaleOnlyChangesTheScale )
{
	const Transform transform( Vec3( 1.0f, 2.0f, 3.0f ), QuatFromEuler( Vec3( 10.0f, 20.0f, 30.0f ) ), Vec3( 1.0f, 1.0f, 1.0f ) );

	Transform scaled = transform;
	scaled.scale     = Vec3( 2.0f, 3.0f, 4.0f );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Scale, transform, scaled.ToMatrix44() );

	EXPECT_NEAR( result.scale.x, 2.0f, 0.001f );
	EXPECT_NEAR( result.scale.y, 3.0f, 0.001f );
	EXPECT_NEAR( result.scale.z, 4.0f, 0.001f );
	EXPECT_EQ( result.position, transform.position );
	ExpectSameRotation( result.rotation, transform.rotation );
}

TEST( GizmoMatrixTests, ScaleKeepsAMirroredAxis )
{
	const Transform transform( Vec3(), Quat(), Vec3( -1.0f, 1.0f, 1.0f ) );

	Transform scaled = transform;
	scaled.scale     = Vec3( -3.0f, 1.0f, 1.0f );

	const Transform result = ApplyGizmoMatrix( GizmoMode::Scale, transform, scaled.ToMatrix44() );

	EXPECT_NEAR( result.scale.x, -3.0f, 0.001f );
}

TEST( RendererCameraTests, NoCameraWithoutARegisteredOne )
{
	const Renderer renderer;

	EXPECT_EQ( renderer.GetCamera(), nullptr );
}

TEST( EditorDefaultsTests, NoLevelMeansNoSelection )
{
	Editor editor;

	EXPECT_EQ( editor.GetLevel(), nullptr );
	EXPECT_EQ( editor.GetSelectedEntity(), nullptr );
}

TEST_F( EditorTests, GetLevelReturnsTheSessionLevel )
{
	Editor editor;

	EXPECT_EQ( editor.GetLevel(), GetSessionLevel() );
}

TEST_F( EditorTests, NothingIsSelectedByDefault )
{
	GetSessionLevel()->CreateEntity( "Entity" );

	Editor editor;

	EXPECT_EQ( editor.GetSelectedEntity(), nullptr );
}

TEST_F( EditorTests, SelectEntityRemembersTheEntity )
{
	Entity* entity = GetSessionLevel()->CreateEntity( "Entity" );

	Editor editor;
	editor.SelectEntity( entity );

	EXPECT_EQ( editor.GetSelectedEntity(), entity );
}

TEST_F( EditorTests, SelectingNullClearsTheSelection )
{
	Entity* entity = GetSessionLevel()->CreateEntity( "Entity" );

	Editor editor;
	editor.SelectEntity( entity );
	editor.SelectEntity( nullptr );

	EXPECT_EQ( editor.GetSelectedEntity(), nullptr );
}

TEST_F( EditorTests, RemovingTheSelectedEntityClearsTheSelection )
{
	Entity* entity = GetSessionLevel()->CreateEntity( "Entity" );

	Editor editor;
	editor.SelectEntity( entity );
	ASSERT_TRUE( GetSessionLevel()->RemoveEntity( entity->GetId() ) );

	EXPECT_EQ( editor.GetSelectedEntity(), nullptr );
}

TEST_F( EditorTests, GetEntitiesListsEntitiesInCreationOrder )
{
	Entity* first  = GetSessionLevel()->CreateEntity( "First" );
	Entity* second = GetSessionLevel()->CreateEntity( "Second" );

	Editor editor;
	const std::vector<unique<Entity>>& entities = editor.GetLevel()->GetEntities();

	ASSERT_EQ( entities.size(), 2u );
	EXPECT_EQ( entities[0].get(), first );
	EXPECT_EQ( entities[1].get(), second );
}

TEST( InputCaptureTests, CapturedKeysAndButtonsReportAsUp )
{
	Input inputState;

	inputState.SetCaptured( true, true );

	EXPECT_TRUE( inputState.IsKeyUp( eInputKeys::W ) );
	EXPECT_FALSE( inputState.IsKeyDown( eInputKeys::W ) );
	EXPECT_TRUE( inputState.IsMouseButtonUp( eInputMouseButtons::RIGHT ) );
	EXPECT_FALSE( inputState.IsMouseButtonDown( eInputMouseButtons::RIGHT ) );
	EXPECT_FALSE( inputState.IsMouseMoving() );
}

TEST( EditorLevelTests, OpenLevelClearsTheSelection )
{
	ScopedTempDir directory;
	{
		Level saved;
		saved.CreateEntity( "Other" );
		ASSERT_TRUE( saved.SaveAs( directory.PathTo( "other.lvl" ) ) );
	}

	ASSERT_TRUE( App::GetSession().Init( "", "", directory.GetPath() ) );

	Editor editor;
	editor.SelectEntity( App::GetSession().GetCurrentLevel()->CreateEntity( "Selected" ) );

	EXPECT_TRUE( editor.OpenLevel( "other" ) );
	EXPECT_EQ( editor.GetSelectedEntity(), nullptr );

	App::GetSession().Shutdown();
}

TEST( EditorLevelTests, FailedOpenKeepsTheSelection )
{
	ScopedTempDir directory;
	ASSERT_TRUE( App::GetSession().Init( "", "", directory.GetPath() ) );

	Entity* entity = App::GetSession().GetCurrentLevel()->CreateEntity( "Selected" );

	Editor editor;
	editor.SelectEntity( entity );

	EXPECT_FALSE( editor.OpenLevel( "missing" ) );
	EXPECT_EQ( editor.GetSelectedEntity(), entity );

	App::GetSession().Shutdown();
}

TEST( EditorLevelTests, SaveLevelAsAddsTheLevelExtension )
{
	ScopedTempDir directory;
	ASSERT_TRUE( App::GetSession().Init( "", "", directory.GetPath() ) );

	Editor editor;

	EXPECT_TRUE( editor.SaveLevelAs( "named" ) );
	EXPECT_TRUE( std::filesystem::exists( directory.PathTo( "levels/named.lvl" ) ) );

	App::GetSession().Shutdown();
}

TEST( EditorLevelTests, SaveLevelAsKeepsAnExistingExtension )
{
	ScopedTempDir directory;
	ASSERT_TRUE( App::GetSession().Init( "", "", directory.GetPath() ) );

	Editor editor;

	EXPECT_TRUE( editor.SaveLevelAs( "named.level" ) );
	EXPECT_TRUE( std::filesystem::exists( directory.PathTo( "levels/named.level" ) ) );

	App::GetSession().Shutdown();
}

TEST( EditorLevelTests, SaveLevelNeedsANameForNewLevels )
{
	ScopedTempDir directory;
	ASSERT_TRUE( App::GetSession().Init( "", "", directory.GetPath() ) );

	Editor editor;

	EXPECT_FALSE( editor.SaveLevel() );

	App::GetSession().Shutdown();
}
