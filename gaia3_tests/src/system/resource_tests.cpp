#include <gtest/gtest.h>

#include <system/resource.h>

namespace
{
class TestResource : public Resource
{
public:
	explicit TestResource( std::string_view path = "test.resource", ResourceState state = ResourceState::NotLoaded )
	    : Resource( path, state )
	{
	}

	int  loadCount    = 0;
	int  unloadCount  = 0;
	bool bLoadWorks   = true;
	bool bUnloadWorks = true;
	bool bSaveWorks   = true;

protected:
	bool LoadInternal() override
	{
		++loadCount;
		return bLoadWorks;
	}

	bool UnloadInternal() override
	{
		++unloadCount;
		return bUnloadWorks;
	}

	bool SaveAsInternal( std::string_view path ) override { return bSaveWorks; }
};

shared<Resource> CreateNothing( std::string_view path, std::string_view entry )
{
	return nullptr;
}
} // namespace

TEST( ResourceTests, LoadSucceedsWhenAlreadyLoaded )
{
	TestResource resource;

	EXPECT_TRUE( resource.Load() );
	EXPECT_TRUE( resource.Load() );
	EXPECT_TRUE( resource.IsLoaded() );
	EXPECT_EQ( resource.loadCount, 1 );
}

TEST( ResourceTests, LoadFailsWhenLoadInternalFails )
{
	TestResource resource;
	resource.bLoadWorks = false;

	EXPECT_FALSE( resource.Load() );
	EXPECT_FALSE( resource.IsLoaded() );
}

TEST( ResourceTests, FailedLoadCanBeRetried )
{
	TestResource resource;
	resource.bLoadWorks = false;
	EXPECT_FALSE( resource.Load() );
	EXPECT_EQ( resource.GetState(), ResourceState::NotLoaded );

	resource.bLoadWorks = true;
	EXPECT_TRUE( resource.Load() );
	EXPECT_EQ( resource.loadCount, 2 );
}

TEST( ResourceTests, NewResourcesCannotBeLoaded )
{
	TestResource resource( "", ResourceState::New );

	EXPECT_TRUE( resource.IsNew() );
	EXPECT_FALSE( resource.Load() );
	EXPECT_EQ( resource.loadCount, 0 );
	EXPECT_TRUE( resource.IsNew() );
}

TEST( ResourceTests, UnloadReturnsToNotLoaded )
{
	TestResource resource;
	ASSERT_TRUE( resource.Load() );

	EXPECT_TRUE( resource.Unload() );
	EXPECT_EQ( resource.GetState(), ResourceState::NotLoaded );
	EXPECT_EQ( resource.unloadCount, 1 );
}

TEST( ResourceTests, UnloadDoesNothingWhenNotLoaded )
{
	TestResource resource;

	EXPECT_FALSE( resource.Unload() );
	EXPECT_EQ( resource.unloadCount, 0 );
}

TEST( ResourceTests, FailedUnloadStaysLoaded )
{
	TestResource resource;
	ASSERT_TRUE( resource.Load() );
	resource.bUnloadWorks = false;

	EXPECT_FALSE( resource.Unload() );
	EXPECT_TRUE( resource.IsLoaded() );
}

TEST( ResourceTests, UnloadedResourceCanBeLoadedAgain )
{
	TestResource resource;
	ASSERT_TRUE( resource.Load() );
	ASSERT_TRUE( resource.Unload() );

	EXPECT_TRUE( resource.Load() );
	EXPECT_EQ( resource.loadCount, 2 );
}

TEST( ResourceTests, NameIsTheFileStem )
{
	EXPECT_EQ( TestResource( "models/cube.obj" ).GetName(), "cube" );
	EXPECT_EQ( TestResource( "cube" ).GetName(), "cube" );
	EXPECT_EQ( TestResource( "" ).GetName(), "" );
}

TEST( ResourceTests, SetPathUpdatesPathAndName )
{
	TestResource resource( "first.obj" );

	resource.SetPath( "folder/second.obj" );

	EXPECT_EQ( resource.GetPath(), "folder/second.obj" );
	EXPECT_EQ( resource.GetName(), "second" );
}

TEST( ResourceTests, AnEntryNamesTheResource )
{
	TestResource resource( "materials/sponza.mtl" );
	EXPECT_TRUE( resource.GetEntry().empty() );

	resource.SetEntry( "bricks" );

	EXPECT_EQ( resource.GetEntry(), "bricks" );
	EXPECT_EQ( resource.GetName(), "bricks" );
	EXPECT_EQ( resource.GetPath(), "materials/sponza.mtl" );
}

TEST( ResourceTests, MovingAnEntryKeepsItsName )
{
	TestResource resource( "materials/sponza.mtl" );
	resource.SetEntry( "bricks" );

	resource.SetPath( "materials/copy.mtl" );

	EXPECT_EQ( resource.GetName(), "bricks" );
	EXPECT_EQ( resource.GetPath(), "materials/copy.mtl" );
}

TEST( ResourceTests, SaveAsMarksNewResourceAsLoaded )
{
	TestResource resource( "", ResourceState::New );

	EXPECT_TRUE( resource.SaveAs( "saved/file.res" ) );

	EXPECT_TRUE( resource.IsLoaded() );
	EXPECT_EQ( resource.GetPath(), "saved/file.res" );
	EXPECT_EQ( resource.GetName(), "file" );
}

TEST( ResourceTests, FailedSaveAsKeepsStateAndPath )
{
	TestResource resource( "original.res", ResourceState::New );
	resource.bSaveWorks = false;

	EXPECT_FALSE( resource.SaveAs( "other.res" ) );

	EXPECT_TRUE( resource.IsNew() );
	EXPECT_EQ( resource.GetPath(), "original.res" );
}

TEST( ResourceTests, RegisterRejectsIncompleteDescriptors )
{
	EXPECT_FALSE( Resource::RegisterResourceType( Resource::TypeDescriptor{ 0, "bad", { ".bad" }, &CreateNothing } ) );
	EXPECT_FALSE( Resource::RegisterResourceType( Resource::TypeDescriptor{ 1, "", { ".bad" }, &CreateNothing } ) );
	EXPECT_FALSE( Resource::RegisterResourceType( Resource::TypeDescriptor{ 1, "bad", {}, &CreateNothing } ) );
	EXPECT_FALSE( Resource::RegisterResourceType( Resource::TypeDescriptor{ 1, "bad", { ".bad" }, nullptr } ) );

	EXPECT_EQ( Resource::FindResourceTypeByExtension( ".bad" ), nullptr );
}

TEST( ResourceTests, TypesAreFoundByTheirId )
{
	const u64 typeId = 0x5678;
	ASSERT_TRUE( Resource::RegisterResourceType( Resource::TypeDescriptor{ typeId, "byid", { ".byid" }, &CreateNothing } ) );

	const Resource::TypeDescriptor* descriptor = Resource::FindResourceTypeById( typeId );

	ASSERT_NE( descriptor, nullptr );
	EXPECT_EQ( descriptor->Folder, "byid" );
	EXPECT_EQ( Resource::FindResourceTypeById( 0x9999 ), nullptr );
}

TEST( ResourceTests, ExtensionsMatchIgnoringCase )
{
	const u64 typeId = 0x1234;
	ASSERT_TRUE( Resource::RegisterResourceType( Resource::TypeDescriptor{ typeId, "mixed", { ".MixedCase", ".second" }, &CreateNothing } ) );

	const Resource::TypeDescriptor* lowerCase = Resource::FindResourceTypeByExtension( ".mixedcase" );
	const Resource::TypeDescriptor* upperCase = Resource::FindResourceTypeByExtension( ".MIXEDCASE" );
	const Resource::TypeDescriptor* second    = Resource::FindResourceTypeByExtension( ".second" );

	ASSERT_NE( lowerCase, nullptr );
	ASSERT_NE( upperCase, nullptr );
	ASSERT_NE( second, nullptr );
	EXPECT_EQ( lowerCase->TypeId, typeId );
	EXPECT_EQ( upperCase->TypeId, typeId );
	EXPECT_EQ( second->TypeId, typeId );
}

TEST( ResourceTests, UnknownExtensionsAreNotFound )
{
	EXPECT_EQ( Resource::FindResourceTypeByExtension( ".unknownextension" ), nullptr );
}
