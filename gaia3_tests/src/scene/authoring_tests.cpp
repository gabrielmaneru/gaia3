#include "testHelpers.h"

#include "testComponents.h"

#include <core/app.h>

namespace
{
class AuthoringTests : public testing::Test
{
protected:
	void SetUp() override { ASSERT_TRUE( App::GetSession().Init( "", "", m_Directory.GetPath() ) ); }
	void TearDown() override { App::GetSession().Shutdown(); }

	Level* GetSessionLevel() { return App::GetSession().GetCurrentLevel(); }

	ScopedTempDir m_Directory;
	Editor        m_Editor;
};
} // namespace

TEST( EntityAuthoringTests, SetNameChangesTheName )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Before" );

	entity->SetName( "After" );

	EXPECT_EQ( entity->GetName(), "After" );
}

TEST( EntityAuthoringTests, AddComponentByNameCreatesARegisteredComponent )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	Component* component = entity->AddComponentByName( "HealthComponent" );

	ASSERT_NE( component, nullptr );
	EXPECT_EQ( component->GetOwner(), entity );
	EXPECT_EQ( entity->GetComponent<HealthComponent>(), component );
	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 1u );
}

TEST( EntityAuthoringTests, AddComponentByNameRejectsUnknownTypes )
{
	Level      level;
	Entity*    entity = level.CreateEntity( "Entity" );
	LogCapture log;

	EXPECT_EQ( entity->AddComponentByName( "NoSuchComponent" ), nullptr );
	EXPECT_TRUE( entity->GetComponents().empty() );
	EXPECT_TRUE( log.Contains( "error: Entity Entity: failed to add component NoSuchComponent" ) );
}

TEST( EntityAuthoringTests, ComponentTypesAreListedInOrder )
{
	const std::vector<std::string> typeNames = ComponentFactory::GetRegisteredTypeNames();

	EXPECT_TRUE( std::is_sorted( typeNames.begin(), typeNames.end() ) );
	EXPECT_NE( std::find( typeNames.begin(), typeNames.end(), "HealthComponent" ), typeNames.end() );
	EXPECT_NE( std::find( typeNames.begin(), typeNames.end(), "ArmorComponent" ), typeNames.end() );
}

TEST( LevelCloneTests, CloneCopiesTheTransformAndComponents )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );
	source->SetPosition(Vec3( 1.0f, 2.0f, 3.0f ));
	source->AddComponent<HealthComponent>( 42 );
	source->AddComponent<ArmorComponent>();

	Entity* clone = level.CloneEntity( source->GetId() );

	ASSERT_NE( clone, nullptr );
	EXPECT_NE( clone->GetId(), source->GetId() );
	EXPECT_EQ( clone->GetName(), "Source (1)" );
	EXPECT_TRUE( clone->GetPosition() == Vec3( 1.0f, 2.0f, 3.0f ) );
	ASSERT_NE( clone->GetComponent<HealthComponent>(), nullptr );
	EXPECT_EQ( clone->GetComponent<HealthComponent>()->m_Health, 42 );
	EXPECT_EQ( clone->GetComponent<HealthComponent>()->GetOwner(), clone );
	EXPECT_NE( clone->GetComponent<ArmorComponent>(), nullptr );
}

TEST( LevelCloneTests, CloneIsIndependentFromTheSource )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );
	source->AddComponent<HealthComponent>( 42 );

	Entity* clone = level.CloneEntity( source->GetId() );
	ASSERT_NE( clone, nullptr );

	clone->GetComponent<HealthComponent>()->m_Health = 1;
	clone->SetPosition( Vec3( 9.0f, 9.0f, 9.0f ));

	EXPECT_EQ( source->GetComponent<HealthComponent>()->m_Health, 42 );
	EXPECT_TRUE( source->GetPosition() == Vec3::V_Zero );
}

TEST( LevelCloneTests, ClonesCountUpFromTheSameSource )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );

	EXPECT_EQ( level.CloneEntity( source->GetId() )->GetName(), "Source (1)" );
	EXPECT_EQ( level.CloneEntity( source->GetId() )->GetName(), "Source (2)" );
	EXPECT_EQ( level.CloneEntity( source->GetId() )->GetName(), "Source (3)" );
}

TEST( LevelCloneTests, CloneUsesTheFirstFreeNumber )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );

	level.CloneEntity( source->GetId() );
	Entity* second = level.CloneEntity( source->GetId() );
	level.CloneEntity( source->GetId() );

	level.RemoveEntity( second->GetId() );

	EXPECT_EQ( level.CloneEntity( source->GetId() )->GetName(), "Source (2)" );
}

TEST( LevelCloneTests, CloneSkipsNumbersTakenByRenamedEntities )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );
	level.CreateEntity( "Source (1)" );

	EXPECT_EQ( level.CloneEntity( source->GetId() )->GetName(), "Source (2)" );
}

TEST( LevelCloneTests, CloningACloneContinuesTheNumbering )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );

	Entity* clone = level.CloneEntity( source->GetId() );

	EXPECT_EQ( level.CloneEntity( clone->GetId() )->GetName(), "Source (2)" );
}

TEST( LevelCloneTests, CloningACloneWithoutItsSourceReusesItsNumber )
{
	Level   level;
	Entity* source = level.CreateEntity( "Source" );

	Entity* clone = level.CloneEntity( source->GetId() );
	level.RemoveEntity( source->GetId() );
	level.RemoveEntity( clone->GetId() );

	Entity* onlyClone = level.CreateEntity( "Source (4)" );

	EXPECT_EQ( level.CloneEntity( onlyClone->GetId() )->GetName(), "Source (1)" );
}

TEST( LevelCloneTests, CloneKeepsParenthesesThatAreNotANumber )
{
	Level level;

	EXPECT_EQ( level.CloneEntity( level.CreateEntity( "Crate (big)" )->GetId() )->GetName(), "Crate (big) (1)" );
	EXPECT_EQ( level.CloneEntity( level.CreateEntity( "Crate ()" )->GetId() )->GetName(), "Crate () (1)" );
	EXPECT_EQ( level.CloneEntity( level.CreateEntity( "Crate(3)" )->GetId() )->GetName(), "Crate(3) (1)" );
	EXPECT_EQ( level.CloneEntity( level.CreateEntity( " (3)" )->GetId() )->GetName(), " (3) (1)" );
}

TEST( LevelCloneTests, CloningAnEntityWithAnEmptyNameWorks )
{
	Level level;

	EXPECT_EQ( level.CloneEntity( level.CreateEntity( "" )->GetId() )->GetName(), " (1)" );
}

TEST( LevelCloneTests, EntitiesCanShareAName )
{
	Level level;

	const Entity* first  = level.CreateEntity( "Same" );
	const Entity* second = level.CreateEntity( "Same" );

	EXPECT_NE( first->GetId(), second->GetId() );
	EXPECT_EQ( level.FindEntity( first->GetId() ), first );
	EXPECT_EQ( level.FindEntity( second->GetId() ), second );
}

TEST( LevelCloneTests, EntitiesWithTheSameNameSurviveSaving )
{
	ScopedTempDir directory;
	const std::string path = directory.PathTo( "same.lvl" );
	{
		Level saved;
		saved.CreateEntity( "Same" );
		saved.CreateEntity( "Same" );
		ASSERT_TRUE( saved.SaveAs( path ) );
	}

	Level loaded( path );
	ASSERT_TRUE( loaded.Load() );

	ASSERT_EQ( loaded.GetEntities().size(), 2u );
	EXPECT_EQ( loaded.GetEntities()[0]->GetName(), "Same" );
	EXPECT_EQ( loaded.GetEntities()[1]->GetName(), "Same" );
	EXPECT_NE( loaded.GetEntities()[0]->GetId(), loaded.GetEntities()[1]->GetId() );
}

TEST( LevelCloneTests, CloningAnUnknownEntityFails )
{
	Level level;

	EXPECT_EQ( level.CloneEntity( 12345 ), nullptr );
	EXPECT_TRUE( level.GetEntities().empty() );
}

TEST( LevelCloneTests, ClonesAreSavedLikeAnyOtherEntity )
{
	ScopedTempDir directory;
	const std::string path = directory.PathTo( "clones.lvl" );
	{
		Level   saved;
		Entity* source = saved.CreateEntity( "Source" );
		source->AddComponent<HealthComponent>( 42 );
		ASSERT_NE( saved.CloneEntity( source->GetId() ), nullptr );
		ASSERT_TRUE( saved.SaveAs( path ) );
	}

	Level loaded( path );
	ASSERT_TRUE( loaded.Load() );

	EXPECT_EQ( loaded.GetEntities().size(), 2u );
	EXPECT_EQ( loaded.GetComponents<HealthComponent>().size(), 2u );
}

TEST( SessionNewLevelTests, NewLevelReplacesTheCurrentLevel )
{
	ScopedTempDir directory;

	Session session;
	ASSERT_TRUE( session.Init( "", "", directory.GetPath() ) );
	session.GetCurrentLevel()->CreateEntity( "Old" );

	session.NewLevel();

	EXPECT_TRUE( session.GetCurrentLevel()->IsNew() );
	EXPECT_TRUE( session.GetCurrentLevel()->GetEntities().empty() );
}

TEST( SessionNewLevelTests, NewLevelUnloadsALevelFromDisk )
{
	ScopedTempDir directory;
	{
		Level saved;
		saved.CreateEntity( "Saved" );
		ASSERT_TRUE( saved.SaveAs( directory.PathTo( "start.lvl" ) ) );
	}

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );
	const shared<Level> previous = session.GetResourceManager().FindResource<Level>( "start" );

	session.NewLevel();

	EXPECT_FALSE( previous->IsLoaded() );
	EXPECT_TRUE( previous->GetEntities().empty() );
}

TEST_F( AuthoringTests, CreateEntitySelectsTheNewEntity )
{
	Entity* entity = m_Editor.CreateEntity( "Created" );

	ASSERT_NE( entity, nullptr );
	EXPECT_EQ( entity->GetName(), "Created" );
	EXPECT_EQ( m_Editor.GetSelectedEntity(), entity );
}

TEST_F( AuthoringTests, CloneEntitySelectsTheClone )
{
	const Entity* source = m_Editor.CreateEntity( "Source" );

	Entity* clone = m_Editor.CloneEntity( source->GetId() );

	ASSERT_NE( clone, nullptr );
	EXPECT_EQ( m_Editor.GetSelectedEntity(), clone );
}

TEST_F( AuthoringTests, SelectionTellsApartEntitiesWithTheSameName )
{
	Entity* first  = m_Editor.CreateEntity( "Same" );
	Entity* second = m_Editor.CreateEntity( "Same" );

	m_Editor.SelectEntity( first );
	EXPECT_EQ( m_Editor.GetSelectedEntity(), first );

	m_Editor.SelectEntity( second );
	EXPECT_EQ( m_Editor.GetSelectedEntity(), second );
}

TEST_F( AuthoringTests, FailedCloneKeepsTheSelection )
{
	Entity* entity = m_Editor.CreateEntity( "Source" );

	EXPECT_EQ( m_Editor.CloneEntity( 12345 ), nullptr );
	EXPECT_EQ( m_Editor.GetSelectedEntity(), entity );
}

TEST_F( AuthoringTests, DeleteEntityRemovesItAndClearsTheSelection )
{
	const Entity* entity = m_Editor.CreateEntity( "Deleted" );

	EXPECT_TRUE( m_Editor.DeleteEntity( entity->GetId() ) );

	EXPECT_EQ( m_Editor.GetSelectedEntity(), nullptr );
	EXPECT_TRUE( GetSessionLevel()->GetEntities().empty() );
}

TEST_F( AuthoringTests, DeletingAnUnknownEntityFails )
{
	EXPECT_FALSE( m_Editor.DeleteEntity( 12345 ) );
}

TEST_F( AuthoringTests, NewLevelClearsTheSelection )
{
	m_Editor.CreateEntity( "Selected" );

	m_Editor.NewLevel();

	EXPECT_EQ( m_Editor.GetSelectedEntity(), nullptr );
	EXPECT_TRUE( m_Editor.GetLevel()->IsNew() );
}

TEST_F( AuthoringTests, AnAuthoredLevelSurvivesSavingAndLoading )
{
	Entity* entity = m_Editor.CreateEntity( "Hero" );
	entity->SetName( "Renamed" );
	entity->SetPosition( Vec3( 1.0f, 2.0f, 3.0f ));
	entity->AddComponentByName( "HealthComponent" );

	ASSERT_TRUE( m_Editor.SaveLevelAs( "authored" ) );
	m_Editor.NewLevel();
	ASSERT_TRUE( m_Editor.OpenLevel( "authored" ) );

	ASSERT_EQ( m_Editor.GetLevel()->GetEntities().size(), 1u );
	const unique<Entity>& loaded = m_Editor.GetLevel()->GetEntities()[0];
	EXPECT_EQ( loaded->GetName(), "Renamed" );
	EXPECT_TRUE( loaded->GetPosition() == Vec3( 1.0f, 2.0f, 3.0f ) );
	EXPECT_NE( loaded->GetComponent<HealthComponent>(), nullptr );
}
