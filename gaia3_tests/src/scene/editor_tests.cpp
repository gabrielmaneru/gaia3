#include "testHelpers.h"

#include "testComponents.h"

#include <core/app.h>

namespace
{
class EditorTests : public testing::Test
{
protected:
	void SetUp() override { ASSERT_TRUE( App::GetSession().Init( "", "", "" ) ); }
	void TearDown() override { App::GetSession().Shutdown(); }

	Level* GetSessionLevel() { return App::GetSession().GetCurrentLevel(); }
};

class UnsupportedPropertyHolder
{
	RTTI_DECLARE_TYPE( UnsupportedPropertyHolder )
public:
	int              first = 1;
	std::vector<int> unsupported;
};

RTTI_DEFINE_TYPE( UnsupportedPropertyHolder,
                  RTTI_PROPERTY( first ),
                  RTTI_PROPERTY( unsupported ) )
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

TEST_F( EditorTests, GetPropertiesReadsEntityPropertiesInDeclaredOrder )
{
	Entity* entity = GetSessionLevel()->CreateEntity( "Hero" );
	entity->GetTransformRef().position = Vec3( 1.0f, 2.0f, 3.0f );

	const std::vector<EditorProperty> properties = Editor::GetProperties( entity, Entity::StaticTypeInfo() );

	ASSERT_EQ( properties.size(), 2u );
	EXPECT_EQ( properties[0].Name, "m_Name" );
	EXPECT_EQ( properties[0].Value.asString(), "Hero" );
	EXPECT_EQ( properties[1].Name, "m_Transform" );
	EXPECT_EQ( properties[1].Value["position"]["y"].asFloat(), 2.0f );
}

TEST_F( EditorTests, GetPropertiesReadsComponentProperties )
{
	Entity* entity = GetSessionLevel()->CreateEntity( "Hero" );

	shared<HealthComponent> health = entity->AddComponent<HealthComponent>( 42 );

	const std::vector<EditorProperty> properties = Editor::GetProperties( health.get(), health->GetTypeInfo() );

	ASSERT_EQ( properties.size(), 1u );
	EXPECT_EQ( properties[0].Name, "m_Health" );
	EXPECT_EQ( properties[0].Value.asInt(), 42 );
}

TEST( EditorPropertiesTests, UnsupportedPropertiesStayNullAndKeepTheRest )
{
	const UnsupportedPropertyHolder holder;

	const std::vector<EditorProperty> properties = Editor::GetProperties( &holder, UnsupportedPropertyHolder::StaticTypeInfo() );

	ASSERT_EQ( properties.size(), 2u );
	EXPECT_EQ( properties[0].Value.asInt(), 1 );
	EXPECT_TRUE( properties[1].Value.isNull() );
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
	EXPECT_TRUE( std::filesystem::exists( directory.PathTo( "named.lvl" ) ) );

	App::GetSession().Shutdown();
}

TEST( EditorLevelTests, SaveLevelAsKeepsAnExistingExtension )
{
	ScopedTempDir directory;
	ASSERT_TRUE( App::GetSession().Init( "", "", directory.GetPath() ) );

	Editor editor;

	EXPECT_TRUE( editor.SaveLevelAs( "named.level" ) );
	EXPECT_TRUE( std::filesystem::exists( directory.PathTo( "named.level" ) ) );

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
