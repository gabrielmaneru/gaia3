#include "testHelpers.h"

#include "testComponents.h"

#include <scene/level.h>

#include <filesystem>

namespace
{
std::string SaveTestLevel( const ScopedTempDir& directory )
{
	Level level;

	Entity* player = level.CreateEntity( "Player" );
	player->SetPosition( Vec3( 1.0f, 2.0f, 3.0f ) );
	player->AddComponent<HealthComponent>( 42 );
	player->AddComponent<ArmorComponent>();

	level.CreateEntity( "Empty" );

	const std::string path = directory.PathTo( "test.lvl" );
	EXPECT_TRUE( level.SaveAs( path ) );
	return path;
}
} // namespace

TEST( LevelTests, DefaultLevelIsNew )
{
	Level level;

	EXPECT_TRUE( level.IsNew() );
}

TEST( LevelTests, CreateEntityAssignsUniqueIdsAndNames )
{
	Level level;

	Entity* first  = level.CreateEntity( "First" );
	Entity* second = level.CreateEntity( "Second" );

	EXPECT_NE( first->GetId(), second->GetId() );
	EXPECT_EQ( first->GetName(), "First" );
	EXPECT_EQ( second->GetLevel(), &level );
}

TEST( LevelTests, FindEntityById )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	const Level& constLevel = level;
	EXPECT_EQ( level.FindEntity( entity->GetId() ), entity );
	EXPECT_EQ( constLevel.FindEntity( entity->GetId() ), entity );
	EXPECT_EQ( level.FindEntity( 12345 ), nullptr );
}

TEST( LevelTests, RemoveEntityRemovesOnlyThatEntity )
{
	Level   level;
	Entity* removed = level.CreateEntity( "Removed" );
	Entity* kept    = level.CreateEntity( "Kept" );

	const u64 removedId = removed->GetId();

	EXPECT_TRUE( level.RemoveEntity( removedId ) );

	EXPECT_EQ( level.FindEntity( removedId ), nullptr );
	EXPECT_EQ( level.FindEntity( kept->GetId() ), kept );
}

TEST( LevelTests, RemovingAnUnknownEntityFails )
{
	Level level;

	EXPECT_FALSE( level.RemoveEntity( 12345 ) );
}

TEST( LevelTests, ClearEntitiesRemovesEverythingAndRestartsIds )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	const u64               firstId   = entity->GetId();
	shared<HealthComponent> component = entity->AddComponent<HealthComponent>();

	level.ClearEntities();

	EXPECT_EQ( component->removeCount, 1 );
	EXPECT_TRUE( level.GetComponents<HealthComponent>().empty() );
	EXPECT_EQ( level.CreateEntity( "Next" )->GetId(), firstId );
}

TEST( LevelTests, UpdateReachesEveryEntity )
{
	Level level;

	shared<HealthComponent> first  = level.CreateEntity( "First" )->AddComponent<HealthComponent>();
	shared<HealthComponent> second = level.CreateEntity( "Second" )->AddComponent<HealthComponent>();

	level.Update();

	EXPECT_EQ( first->updateCount, 1 );
	EXPECT_EQ( second->updateCount, 1 );
}

TEST( LevelTests, GetComponentsSpansAllEntities )
{
	Level level;

	level.CreateEntity( "First" )->AddComponent<HealthComponent>();
	level.CreateEntity( "Second" )->AddComponent<HealthComponent>();
	level.CreateEntity( "Third" )->AddComponent<ArmorComponent>();

	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 2u );
	EXPECT_EQ( level.GetComponents<ArmorComponent>().size(), 1u );
}

TEST( LevelTests, SaveAsMarksTheLevelAsLoaded )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "level.lvl" );

	Level level;
	ASSERT_TRUE( level.SaveAs( path ) );

	EXPECT_FALSE( level.IsNew() );
	EXPECT_EQ( level.GetPath(), path );
	EXPECT_TRUE( std::filesystem::exists( path ) );
}

TEST( LevelTests, SaveAsCreatesMissingFolders )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "nested/folder/level.lvl" );

	Level level;

	EXPECT_TRUE( level.SaveAs( path ) );
	EXPECT_TRUE( std::filesystem::exists( path ) );
}

TEST( LevelTests, SaveAsRejectsEmptyPath )
{
	Level level;

	EXPECT_FALSE( level.SaveAs( "" ) );
	EXPECT_TRUE( level.IsNew() );
}

TEST( LevelTests, SaveAndLoadRestoresEntitiesAndComponents )
{
	ScopedTempDir     directory;
	const std::string path = SaveTestLevel( directory );

	Level level( path );
	ASSERT_TRUE( level.Load() );

	Entity* player = level.FindEntity( 1 );
	ASSERT_NE( player, nullptr );
	EXPECT_EQ( player->GetName(), "Player" );
	EXPECT_EQ( player->GetLevel(), &level );
	EXPECT_TRUE( player->GetPosition() == Vec3( 1.0f, 2.0f, 3.0f ) );

	HealthComponent* health = player->GetComponent<HealthComponent>();
	ASSERT_NE( health, nullptr );
	EXPECT_EQ( health->m_Health, 42 );
	EXPECT_EQ( health->GetOwner(), player );
	EXPECT_NE( player->GetComponent<ArmorComponent>(), nullptr );

	Entity* empty = level.FindEntity( 2 );
	ASSERT_NE( empty, nullptr );
	EXPECT_EQ( empty->GetName(), "Empty" );
	EXPECT_TRUE( empty->GetComponents().empty() );

	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 1u );
}

TEST( LevelTests, EmptyLevelRoundTrips )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "empty.lvl" );

	Level saved;
	ASSERT_TRUE( saved.SaveAs( path ) );

	Level level( path );
	EXPECT_TRUE( level.Load() );
	EXPECT_EQ( level.FindEntity( 1 ), nullptr );
}

TEST( LevelTests, EntitiesCreatedAfterLoadGetNewIds )
{
	ScopedTempDir     directory;
	const std::string path = SaveTestLevel( directory );

	Level level( path );
	ASSERT_TRUE( level.Load() );

	const Entity* created = level.CreateEntity( "Created" );

	EXPECT_NE( created->GetId(), 1u );
	EXPECT_NE( created->GetId(), 2u );
}

TEST( LevelTests, ReloadingReplacesEntitiesInsteadOfDuplicating )
{
	ScopedTempDir     directory;
	const std::string path = SaveTestLevel( directory );

	Level level( path );
	ASSERT_TRUE( level.Load() );
	ASSERT_TRUE( level.Unload() );
	ASSERT_TRUE( level.Load() );

	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 1u );
	EXPECT_NE( level.FindEntity( 2 ), nullptr );
	EXPECT_EQ( level.FindEntity( 3 ), nullptr );
}

TEST( LevelTests, LoadFailsForMissingFile )
{
	ScopedTempDir directory;

	Level level( directory.PathTo( "missing.lvl" ) );

	EXPECT_FALSE( level.Load() );
	EXPECT_FALSE( level.IsLoaded() );
}

TEST( LevelTests, LoadFailsForInvalidJson )
{
	ScopedTempDir directory;

	Level level( directory.Write( "bad.lvl", "{ this is not json" ) );

	EXPECT_FALSE( level.Load() );
}

TEST( LevelTests, LoadFailsWithoutEntityList )
{
	ScopedTempDir directory;

	Level level( directory.Write( "bad.lvl", "{}" ) );

	EXPECT_FALSE( level.Load() );
}

TEST( LevelTests, LoadFailsWhenRootIsNotAnObject )
{
	ScopedTempDir directory;

	Level level( directory.Write( "bad.lvl", "[]" ) );

	EXPECT_FALSE( level.Load() );
}

TEST( LevelTests, LoadFailsForOutOfRangeNumbers )
{
	ScopedTempDir directory;

	Level level( directory.Write( "bad.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Entity",
			"m_SerializedComponents": [ { "m_TypeName": "HealthComponent", "m_Data": { "m_Health": 18446744073709551615 } } ]
		} ]
	})" ) );

	EXPECT_FALSE( level.Load() );
}

TEST( LevelTests, LoadFailsForUnknownComponentTypes )
{
	ScopedTempDir directory;

	Level level( directory.Write( "bad.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Entity",
			"m_SerializedComponents": [ { "m_TypeName": "NoSuchComponent", "m_Data": {} } ]
		} ]
	})" ) );

	EXPECT_FALSE( level.Load() );
}

TEST( LevelTests, UnloadRemovesEntitiesAndNotifiesComponents )
{
	ScopedTempDir     directory;
	const std::string path = SaveTestLevel( directory );

	Level level( path );
	ASSERT_TRUE( level.Load() );
	shared<HealthComponent> health = level.GetComponents<HealthComponent>()[0];

	EXPECT_TRUE( level.Unload() );

	EXPECT_TRUE( level.GetEntities().empty() );
	EXPECT_EQ( health->removeCount, 1 );
	EXPECT_TRUE( level.GetComponents<HealthComponent>().empty() );
}
