#include "testHelpers.h"

#include <system/resourceManager.h>

namespace
{
// Note: Files named "broken" fail to load, so tests can trigger a load failure from disk
class ManagedResource : public Resource
{
	RTTI_DECLARE_RESOURCE( ManagedResource )
public:
	explicit ManagedResource( std::string_view path, ResourceState state = ResourceState::NotLoaded )
	    : Resource( path, state )
	{
	}

protected:
	bool LoadInternal() override { return GetName() != "broken"; }
};

RTTI_DEFINE_RESOURCE( ManagedResource,
                      RTTI_RESOURCE_FOLDER( "managed" ),
                      RTTI_RESOURCE_EXTENSIONS( ".managed" ) )

class OtherManagedResource : public Resource
{
	RTTI_DECLARE_RESOURCE( OtherManagedResource )
public:
	explicit OtherManagedResource( std::string_view path, ResourceState state = ResourceState::NotLoaded )
	    : Resource( path, state )
	{
	}
};

RTTI_DEFINE_RESOURCE( OtherManagedResource,
                      RTTI_RESOURCE_FOLDER( "othermanaged" ),
                      RTTI_RESOURCE_EXTENSIONS( ".othermanaged" ) )
} // namespace

TEST( ResourceManagerTests, FindsResourcesWithOrWithoutExtension )
{
	ScopedTempDir directory;
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_NE( manager.FindResource<ManagedResource>( "item" ), nullptr );
	EXPECT_NE( manager.FindResource<ManagedResource>( "item.managed" ), nullptr );
}

TEST( ResourceManagerTests, ScansSubfoldersAndFindsByRelativePathOrName )
{
	ScopedTempDir directory;
	directory.Write( "models/props/crate.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> byPath = manager.FindResource<ManagedResource>( "models/props/crate" );
	shared<ManagedResource> byName = manager.FindResource<ManagedResource>( "crate.managed" );

	ASSERT_NE( byPath, nullptr );
	EXPECT_EQ( byPath, byName );
}

TEST( ResourceManagerTests, ResourcesInTheirFolderAreNamedWithoutIt )
{
	ScopedTempDir directory;
	directory.Write( "managed/item.managed", "" );
	directory.Write( "managed/nested/deep.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "item", "nested/deep" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );
	EXPECT_NE( manager.FindResource<ManagedResource>( "item" ), nullptr );
	EXPECT_NE( manager.FindResource<ManagedResource>( "nested/deep" ), nullptr );
}

TEST( ResourceManagerTests, TheTypeFolderCanStillBeWrittenWhenFinding )
{
	ScopedTempDir directory;
	directory.Write( "managed/item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_EQ( manager.FindResource<ManagedResource>( "managed/item" ), resource );
	EXPECT_EQ( manager.FindResource<ManagedResource>( "managed/item.managed" ), resource );
}

TEST( ResourceManagerTests, ResourcesInAnotherFolderKeepTheirWholePath )
{
	ScopedTempDir directory;
	directory.Write( "othermanaged/stray.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "othermanaged/stray" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );
	EXPECT_TRUE( manager.FindResource<ManagedResource>( "othermanaged/stray", true )->IsLoaded() );
}

TEST( ResourceManagerTests, TheTypeFolderWinsOverAFileWithTheSameNameInAnotherFolder )
{
	ScopedTempDir     directory;
	const std::string inFolder = directory.Write( "managed/othermanaged/foo.managed", "" );
	const std::string stray    = directory.Write( "othermanaged/foo.managed", "" );

	LogCapture log;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "othermanaged/foo" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "othermanaged/foo", true );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( std::filesystem::equivalent( resource->GetPath(), inFolder ) );
	EXPECT_EQ( manager.FindResource<ManagedResource>( "foo" ), resource );
	EXPECT_TRUE( log.Contains( "warning: ResourceManager: ignoring" ) );
	EXPECT_TRUE( log.Contains( std::filesystem::path( stray ).filename().string() ) );
}

TEST( ResourceManagerTests, TheTypeFolderWinsOverAFileInTheRoot )
{
	ScopedTempDir     directory;
	const std::string inFolder = directory.Write( "managed/item.managed", "" );
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( std::filesystem::equivalent( resource->GetPath(), inFolder ) );
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>().size(), 1u );
}

TEST( ResourceManagerTests, ACommonFileInTheTypeFolderWinsOverAnAppFileOutsideIt )
{
	ScopedTempDir     directory;
	const std::string inFolder = directory.Write( "common/managed/item.managed", "" );
	directory.Write( "app/item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( directory.PathTo( "common" ), directory.PathTo( "app" ) ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( std::filesystem::equivalent( resource->GetPath(), inFolder ) );
}

TEST( ResourceManagerTests, AnAppFileInTheTypeFolderWinsOverACommonFileOutsideIt )
{
	ScopedTempDir directory;
	directory.Write( "common/item.managed", "" );
	const std::string inFolder = directory.Write( "app/managed/item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( directory.PathTo( "common" ), directory.PathTo( "app" ) ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( std::filesystem::equivalent( resource->GetPath(), inFolder ) );
}

TEST( ResourceManagerTests, AppOverridingCommonInsideTheTypeFolderIsNotAWarning )
{
	ScopedTempDir directory;
	directory.Write( "common/managed/item.managed", "" );
	const std::string appFile = directory.Write( "app/managed/item.managed", "" );

	LogCapture log;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( directory.PathTo( "common" ), directory.PathTo( "app" ) ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( std::filesystem::equivalent( resource->GetPath(), appFile ) );
	EXPECT_FALSE( log.Contains( "warning" ) );
}

TEST( ResourceManagerTests, OnlyTheWholeFolderNameIsTrimmed )
{
	ScopedTempDir directory;
	directory.Write( "managed_extra/item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "managed_extra/item" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );
}

TEST( ResourceManagerTests, SavingWritesIntoTheTypeFolder )
{
	ScopedTempDir directory;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = new_shared<ManagedResource>( "", ResourceState::New );
	ASSERT_TRUE( manager.SaveResourceAs<ManagedResource>( resource, "saved.managed" ) );

	EXPECT_TRUE( std::filesystem::is_directory( directory.PathTo( "managed" ) ) );
	EXPECT_EQ( std::filesystem::path( resource->GetPath() ), std::filesystem::path( directory.GetPath() ) / "managed" / "saved.managed" );

	const std::vector<std::string> expected = { "saved" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );
}

TEST( ResourceManagerTests, ExtensionsMatchIgnoringCaseWhenScanning )
{
	ScopedTempDir directory;
	directory.Write( "upper.MANAGED", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_NE( manager.FindResource<ManagedResource>( "upper" ), nullptr );
}

TEST( ResourceManagerTests, IgnoresFilesWithUnknownExtensions )
{
	ScopedTempDir directory;
	directory.Write( "notes.unknownextension", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "notes" ), nullptr );
}

TEST( ResourceManagerTests, MissingFoldersAreNotAnError )
{
	ScopedTempDir directory;

	ResourceManager manager;
	EXPECT_TRUE( manager.Init( directory.PathTo( "missing_common" ), directory.PathTo( "missing_app" ) ) );
	EXPECT_EQ( manager.FindResource<ManagedResource>( "anything" ), nullptr );
}

TEST( ResourceManagerTests, UnknownResourcesAreNotFound )
{
	ScopedTempDir directory;
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "other" ), nullptr );
}

TEST( ResourceManagerTests, ResourcesAreSeparatedByType )
{
	ScopedTempDir directory;
	directory.Write( "shared.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_NE( manager.FindResource<ManagedResource>( "shared" ), nullptr );
	EXPECT_EQ( manager.FindResource<OtherManagedResource>( "shared" ), nullptr );
}

TEST( ResourceManagerTests, AppResourcesOverrideCommonResources )
{
	ScopedTempDir directory;
	directory.Write( "common/item.managed", "" );
	const std::string appFile = directory.Write( "app/item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( directory.PathTo( "common" ), directory.PathTo( "app" ) ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( std::filesystem::equivalent( resource->GetPath(), appFile ) );
}

TEST( ResourceManagerTests, ResourcesStayUnloadedWithoutForceLoad )
{
	ScopedTempDir directory;
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item" );
	ASSERT_NE( resource, nullptr );
	EXPECT_FALSE( resource->IsLoaded() );
}

TEST( ResourceManagerTests, ForceLoadLoadsTheResource )
{
	ScopedTempDir directory;
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item", true );
	ASSERT_NE( resource, nullptr );
	EXPECT_TRUE( resource->IsLoaded() );
}

TEST( ResourceManagerTests, ForceLoadReturnsTheSameResourceEveryTime )
{
	ScopedTempDir directory;
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> first  = manager.FindResource<ManagedResource>( "item", true );
	shared<ManagedResource> second = manager.FindResource<ManagedResource>( "item", true );

	ASSERT_NE( first, nullptr );
	EXPECT_EQ( first, second );
}

TEST( ResourceManagerTests, ForceLoadReturnsNullWhenLoadingFails )
{
	ScopedTempDir directory;
	directory.Write( "broken.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "broken", true ), nullptr );
	EXPECT_NE( manager.FindResource<ManagedResource>( "broken" ), nullptr );
}

TEST( ResourceManagerTests, ShutdownUnloadsAndForgetsResources )
{
	ScopedTempDir directory;
	directory.Write( "item.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "item", true );
	ASSERT_NE( resource, nullptr );

	manager.Shutdown();

	EXPECT_FALSE( resource->IsLoaded() );
	EXPECT_EQ( manager.FindResource<ManagedResource>( "item" ), nullptr );
}

TEST( ResourceManagerTests, InitForgetsThePreviousScan )
{
	ScopedTempDir directory;
	directory.Write( "first/one.managed", "" );
	directory.Write( "second/two.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.PathTo( "first" ) ) );
	ASSERT_TRUE( manager.Init( "", directory.PathTo( "second" ) ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "one" ), nullptr );
	EXPECT_NE( manager.FindResource<ManagedResource>( "two" ), nullptr );
}

TEST( ResourceManagerTests, SavedResourcesCanBeFoundByTheirNewName )
{
	ScopedTempDir directory;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = new_shared<ManagedResource>( "", ResourceState::New );
	ASSERT_TRUE( manager.SaveResourceAs<ManagedResource>( resource, "saved.managed" ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "saved" ), resource );
	EXPECT_TRUE( resource->IsLoaded() );
}

TEST( ResourceManagerTests, SavingUnderANewNameKeepsTheOldFileAvailable )
{
	ScopedTempDir     directory;
	const std::string oldFile = directory.Write( "old.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "old", true );
	ASSERT_NE( resource, nullptr );
	ASSERT_TRUE( manager.SaveResourceAs<ManagedResource>( resource, "new.managed" ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "new" ), resource );

	shared<ManagedResource> oldResource = manager.FindResource<ManagedResource>( "old" );
	ASSERT_NE( oldResource, nullptr );
	EXPECT_NE( oldResource, resource );
	EXPECT_FALSE( oldResource->IsLoaded() );
	EXPECT_TRUE( std::filesystem::equivalent( oldResource->GetPath(), oldFile ) );
}

TEST( ResourceManagerTests, SavingOverTheSameNameKeepsOneResource )
{
	ScopedTempDir directory;
	directory.Write( "same.managed", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = manager.FindResource<ManagedResource>( "same", true );
	ASSERT_NE( resource, nullptr );
	ASSERT_TRUE( manager.SaveResourceAs<ManagedResource>( resource, "same.managed" ) );

	EXPECT_EQ( manager.FindResource<ManagedResource>( "same" ), resource );
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>().size(), 1u );
}

TEST( ResourceManagerTests, SaveFailsWithoutResourceNameOrAppPath )
{
	ScopedTempDir           directory;
	shared<ManagedResource> resource = new_shared<ManagedResource>( "", ResourceState::New );

	ResourceManager noAppPath;
	ASSERT_TRUE( noAppPath.Init( "", "" ) );
	EXPECT_FALSE( noAppPath.SaveResourceAs<ManagedResource>( resource, "file.managed" ) );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );
	EXPECT_FALSE( manager.SaveResourceAs<ManagedResource>( nullptr, "file.managed" ) );
	EXPECT_FALSE( manager.SaveResourceAs<ManagedResource>( resource, "" ) );
}

TEST( ResourceManagerTests, GetResourceNamesListsOneTypeInOrder )
{
	ScopedTempDir directory;
	directory.Write( "zebra.managed", "" );
	directory.Write( "nested/apple.managed", "" );
	directory.Write( "other.othermanaged", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "nested/apple", "zebra" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );
}

TEST( ResourceManagerTests, GetResourceNamesIsEmptyForUnusedTypes )
{
	ResourceManager manager;

	EXPECT_TRUE( manager.GetResourceNames<ManagedResource>().empty() );
}

TEST( ResourceManagerTests, SavedResourcesAppearInTheNames )
{
	ScopedTempDir directory;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	shared<ManagedResource> resource = new_shared<ManagedResource>( "", ResourceState::New );
	ASSERT_TRUE( manager.SaveResourceAs<ManagedResource>( resource, "saved.managed" ) );

	const std::vector<std::string> expected = { "saved" };
	EXPECT_EQ( manager.GetResourceNames<ManagedResource>(), expected );
}
