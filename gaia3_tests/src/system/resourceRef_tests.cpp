#include "testHelpers.h"

#include <system/resourceRef.h>
#include <system/rtti.h>

namespace
{
class ReferencedResource : public Resource
{
	RTTI_DECLARE_RESOURCE( ReferencedResource )
public:
	explicit ReferencedResource( std::string_view path, ResourceState state = ResourceState::NotLoaded )
	    : Resource( path, state )
	{
	}
};

RTTI_DEFINE_RESOURCE( ReferencedResource,
                      RTTI_RESOURCE_EXTENSIONS( ".referenced" ) )

class ReferenceHolder
{
	RTTI_DECLARE_TYPE( ReferenceHolder )
public:
	ResourceRef<ReferencedResource> reference;
	int                             plainValue = 0;
};

RTTI_DEFINE_TYPE( ReferenceHolder,
                  RTTI_PROPERTY( reference ),
                  RTTI_PROPERTY( plainValue ) )
} // namespace

TEST( ResourceRefTests, StartsEmpty )
{
	const ResourceRef<ReferencedResource> reference;

	EXPECT_TRUE( reference.GetName().empty() );
	EXPECT_EQ( reference.Get(), nullptr );
}

TEST( ResourceRefTests, SavesOnlyTheResourceName )
{
	ReferenceHolder holder;
	holder.reference.SetName( "models/cube" );

	Json::Value json;
	ASSERT_TRUE( rtti::Serialize( holder, json ) );

	EXPECT_TRUE( json["reference"].isString() );
	EXPECT_EQ( json["reference"].asString(), "models/cube" );
}

TEST( ResourceRefTests, RestoresTheResourceName )
{
	Json::Value json( Json::objectValue );
	json["reference"] = "models/cube";

	ReferenceHolder holder;
	ASSERT_TRUE( rtti::Deserialize( holder, json ) );

	EXPECT_EQ( holder.reference.GetName(), "models/cube" );
}

TEST( ResourceRefTests, RejectsValuesThatAreNotNames )
{
	Json::Value json( Json::objectValue );
	json["reference"] = 5;

	ReferenceHolder holder;

	EXPECT_FALSE( rtti::Deserialize( holder, json ) );
}

TEST( ResourceRefTests, PropertyKnowsItsResourceType )
{
	const rtti::TypeInfo& typeInfo = ReferenceHolder::StaticTypeInfo();

	EXPECT_EQ( typeInfo.FindProperty( "reference" )->ResourceTypeId, GetTypeId<ReferencedResource>() );
	EXPECT_EQ( typeInfo.FindProperty( "plainValue" )->ResourceTypeId, 0u );
}

TEST( ResourceRefTests, ResolveLoadsTheResource )
{
	ScopedTempDir directory;
	directory.Write( "item.referenced", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	ResourceRef<ReferencedResource> reference( "item" );

	EXPECT_TRUE( reference.Resolve( manager ) );
	ASSERT_NE( reference.Get(), nullptr );
	EXPECT_TRUE( reference.Get()->IsLoaded() );
}

TEST( ResourceRefTests, ReferencesShareTheSameResource )
{
	ScopedTempDir directory;
	directory.Write( "item.referenced", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	ResourceRef<ReferencedResource> first( "item" );
	ResourceRef<ReferencedResource> second( "item" );
	ASSERT_TRUE( first.Resolve( manager ) );
	ASSERT_TRUE( second.Resolve( manager ) );

	EXPECT_EQ( first.Get(), second.Get() );
}

TEST( ResourceRefTests, EmptyNameResolvesToNoResource )
{
	ScopedTempDir directory;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	ResourceRef<ReferencedResource> reference;

	EXPECT_TRUE( reference.Resolve( manager ) );
	EXPECT_EQ( reference.Get(), nullptr );
}

TEST( ResourceRefTests, MissingResourceFailsToResolve )
{
	ScopedTempDir directory;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	ResourceRef<ReferencedResource> reference( "missing" );

	EXPECT_FALSE( reference.Resolve( manager ) );
	EXPECT_EQ( reference.Get(), nullptr );
}

TEST( ResourceRefTests, ChangingTheNameDropsTheOldResource )
{
	ScopedTempDir directory;
	directory.Write( "item.referenced", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	ResourceRef<ReferencedResource> reference( "item" );
	ASSERT_TRUE( reference.Resolve( manager ) );

	reference.SetName( "other" );

	EXPECT_EQ( reference.Get(), nullptr );
}

TEST( ResourceRefTests, ReleaseKeepsTheName )
{
	ScopedTempDir directory;
	directory.Write( "item.referenced", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	ResourceRef<ReferencedResource> reference( "item" );
	ASSERT_TRUE( reference.Resolve( manager ) );

	reference.Release();

	EXPECT_EQ( reference.Get(), nullptr );
	EXPECT_EQ( reference.GetName(), "item" );
}

TEST( ResourceManagerNamesTests, NamesCanBeListedByTypeId )
{
	ScopedTempDir directory;
	directory.Write( "item.referenced", "" );

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	const std::vector<std::string> expected = { "item" };
	EXPECT_EQ( manager.GetResourceNamesByTypeId( GetTypeId<ReferencedResource>() ), expected );
}
