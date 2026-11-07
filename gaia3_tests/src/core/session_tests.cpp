#include "testHelpers.h"

#include <core/session.h>
#include <gfx/model.h>

#include <filesystem>

namespace
{
void SaveLevelWithEntity( const std::string& path, std::string_view entityName )
{
	Level level;
	level.CreateEntity( entityName );
	ASSERT_TRUE( level.SaveAs( path ) );
}
} // namespace

TEST( SessionTests, MissingStartLevelCreatesANewLevel )
{
	ScopedTempDir directory;

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	ASSERT_NE( session.GetCurrentLevel(), nullptr );
	EXPECT_TRUE( session.GetCurrentLevel()->IsNew() );
}

TEST( SessionTests, ExistingStartLevelIsLoaded )
{
	ScopedTempDir directory;
	{
		Level level;
		level.CreateEntity( "Saved" );
		ASSERT_TRUE( level.SaveAs( directory.PathTo( "start.lvl" ) ) );
	}

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	Level* level = session.GetCurrentLevel();
	ASSERT_NE( level, nullptr );
	EXPECT_TRUE( level->IsLoaded() );
	ASSERT_NE( level->FindEntity( 1 ), nullptr );
	EXPECT_EQ( level->FindEntity( 1 )->GetName(), "Saved" );
}

TEST( SessionTests, UnreadableStartLevelFallsBackToANewLevel )
{
	ScopedTempDir directory;
	directory.Write( "start.lvl", "not json" );

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	ASSERT_NE( session.GetCurrentLevel(), nullptr );
	EXPECT_TRUE( session.GetCurrentLevel()->IsNew() );
}

TEST( SessionTests, SaveCurrentLevelAsWritesAndRegistersTheLevel )
{
	ScopedTempDir directory;

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	EXPECT_TRUE( session.SaveCurrentLevelAs( "saved.lvl" ) );

	EXPECT_FALSE( session.GetCurrentLevel()->IsNew() );
	EXPECT_TRUE( std::filesystem::exists( directory.PathTo( "levels/saved.lvl" ) ) );

	const shared<Level> found = session.GetResourceManager().FindResource<Level>( "saved" );
	EXPECT_EQ( found.get(), session.GetCurrentLevel() );
}

TEST( SessionTests, LevelsInTheLevelsFolderAreFoundByName )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "levels/town.lvl" ), "Saved" );

	Session session;
	ASSERT_TRUE( session.Init( "town", "", directory.GetPath() ) );

	EXPECT_EQ( session.GetCurrentLevel()->GetEntities().size(), 1u );

	const std::vector<std::string> expected = { "town" };
	EXPECT_EQ( session.GetResourceManager().GetResourceNames<Level>(), expected );
}

TEST( SessionTests, LevelsInAnotherFolderLoadAndShowThatFolder )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "models/stray.lvl" ), "Saved" );

	Session session;
	ASSERT_TRUE( session.Init( "", "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "models/stray" };
	EXPECT_EQ( session.GetResourceManager().GetResourceNames<Level>(), expected );

	ASSERT_TRUE( session.OpenLevel( "models/stray" ) );
	EXPECT_EQ( session.GetCurrentLevel()->GetEntities().size(), 1u );
}

TEST( SessionTests, SavingALevelFromAnotherFolderKeepsItThere )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "models/stray.lvl" );
	SaveLevelWithEntity( path, "Saved" );

	Session session;
	ASSERT_TRUE( session.Init( "models/stray", "", directory.GetPath() ) );
	session.GetCurrentLevel()->CreateEntity( "Added" );

	EXPECT_TRUE( session.SaveCurrentLevel() );

	EXPECT_FALSE( std::filesystem::exists( directory.PathTo( "levels/stray.lvl" ) ) );

	Level reloaded( path );
	ASSERT_TRUE( reloaded.Load() );
	EXPECT_EQ( reloaded.GetEntities().size(), 2u );
}

TEST( SessionTests, ModelsInTheModelsFolderAreFoundByName )
{
	ScopedTempDir directory;
	directory.Write( "models/cube.obj", "" );

	Session session;
	ASSERT_TRUE( session.Init( "", "", directory.GetPath() ) );

	ResourceManager& manager = session.GetResourceManager();

	const std::vector<std::string> expected = { "cube" };
	EXPECT_EQ( manager.GetResourceNames<Model>(), expected );
	EXPECT_NE( manager.FindResource<Model>( "cube" ), nullptr );
	EXPECT_EQ( manager.FindResource<Model>( "models/cube" ), manager.FindResource<Model>( "cube" ) );
}

TEST( SessionTests, SaveFailsWithoutALevel )
{
	Session session;

	EXPECT_FALSE( session.SaveCurrentLevelAs( "saved.lvl" ) );
}

TEST( SessionTests, ShutdownReleasesTheLevel )
{
	ScopedTempDir directory;

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	session.Shutdown();

	EXPECT_EQ( session.GetCurrentLevel(), nullptr );
}

TEST( SessionTests, OpenLevelSwitchesTheCurrentLevel )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "other.lvl" ), "Other" );

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	EXPECT_TRUE( session.OpenLevel( "other" ) );

	ASSERT_NE( session.GetCurrentLevel()->FindEntity( 1 ), nullptr );
	EXPECT_EQ( session.GetCurrentLevel()->FindEntity( 1 )->GetName(), "Other" );
}

TEST( SessionTests, OpenLevelUnloadsThePreviousLevel )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "first.lvl" ), "First" );
	SaveLevelWithEntity( directory.PathTo( "second.lvl" ), "Second" );

	Session session;
	ASSERT_TRUE( session.Init( "first.lvl", "", directory.GetPath() ) );
	const shared<Level> first = session.GetResourceManager().FindResource<Level>( "first" );

	ASSERT_TRUE( session.OpenLevel( "second" ) );

	EXPECT_FALSE( first->IsLoaded() );
	EXPECT_TRUE( first->GetEntities().empty() );
}

TEST( SessionTests, PreviousLevelCanBeOpenedAgain )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "first.lvl" ), "First" );
	SaveLevelWithEntity( directory.PathTo( "second.lvl" ), "Second" );

	Session session;
	ASSERT_TRUE( session.Init( "first.lvl", "", directory.GetPath() ) );
	ASSERT_TRUE( session.OpenLevel( "second" ) );

	EXPECT_TRUE( session.OpenLevel( "first" ) );

	ASSERT_NE( session.GetCurrentLevel()->FindEntity( 1 ), nullptr );
	EXPECT_EQ( session.GetCurrentLevel()->FindEntity( 1 )->GetName(), "First" );
}

TEST( SessionTests, OpeningTheCurrentLevelKeepsItsState )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "start.lvl" ), "Saved" );

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );
	session.GetCurrentLevel()->CreateEntity( "Unsaved" );

	EXPECT_TRUE( session.OpenLevel( "start" ) );

	EXPECT_NE( session.GetCurrentLevel()->FindEntity( 2 ), nullptr );
}

TEST( SessionTests, FailedOpenKeepsTheCurrentLevel )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "start.lvl" ), "Saved" );
	directory.Write( "broken.lvl", "not json" );

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	EXPECT_FALSE( session.OpenLevel( "broken" ) );
	EXPECT_FALSE( session.OpenLevel( "missing" ) );

	ASSERT_NE( session.GetCurrentLevel()->FindEntity( 1 ), nullptr );
	EXPECT_EQ( session.GetCurrentLevel()->FindEntity( 1 )->GetName(), "Saved" );
}

TEST( SessionTests, LevelSavedUnderANewNameCanStillBeOpenedFromItsOldFile )
{
	ScopedTempDir directory;
	SaveLevelWithEntity( directory.PathTo( "start.lvl" ), "Original" );

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );
	session.GetCurrentLevel()->CreateEntity( "Added" );
	ASSERT_TRUE( session.SaveCurrentLevelAs( "copy.lvl" ) );

	EXPECT_TRUE( session.OpenLevel( "start" ) );

	EXPECT_EQ( session.GetCurrentLevel()->GetEntities().size(), 1u );
}

TEST( SessionTests, SaveCurrentLevelNeedsAnExistingFile )
{
	ScopedTempDir directory;

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );

	EXPECT_FALSE( session.SaveCurrentLevel() );
	EXPECT_TRUE( session.GetCurrentLevel()->IsNew() );
}

TEST( SessionTests, SaveCurrentLevelWritesChangesToItsFile )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "start.lvl" );
	SaveLevelWithEntity( path, "Saved" );

	Session session;
	ASSERT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );
	session.GetCurrentLevel()->CreateEntity( "Added" );

	EXPECT_TRUE( session.SaveCurrentLevel() );

	Level reloaded( path );
	ASSERT_TRUE( reloaded.Load() );
	EXPECT_EQ( reloaded.GetEntities().size(), 2u );
}
