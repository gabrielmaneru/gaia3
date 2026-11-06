#include "testHelpers.h"

#include <core/session.h>

#include <filesystem>

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
	EXPECT_TRUE( std::filesystem::exists( directory.PathTo( "saved.lvl" ) ) );

	const shared<Level> found = session.GetResourceManager().FindResource<Level>( "saved" );
	EXPECT_EQ( found.get(), session.GetCurrentLevel() );
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
