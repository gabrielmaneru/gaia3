#include <gtest/gtest.h>

#include "testComponents.h"

TEST( ComponentFactoryTests, CreatesRegisteredTypesByName )
{
	ComponentFactory factory;

	shared<Component> component = factory.Create( "HealthComponent" );

	ASSERT_NE( component, nullptr );
	EXPECT_EQ( component->GetTypeInfo().TypeId, HealthComponent::StaticTypeInfo().TypeId );
}

TEST( ComponentFactoryTests, UnknownTypesAreNotCreated )
{
	ComponentFactory factory;

	EXPECT_EQ( factory.Create( "NoSuchComponent" ), nullptr );
}

TEST( ComponentFactoryTests, GetComponentsReturnsOnlyTheRequestedType )
{
	ComponentFactory factory;

	shared<Component> healthA = new_shared<HealthComponent>();
	shared<Component> healthB = new_shared<HealthComponent>();
	shared<Component> armor   = new_shared<ArmorComponent>();
	factory.RegisterComponent( healthA );
	factory.RegisterComponent( healthB );
	factory.RegisterComponent( armor );

	EXPECT_EQ( factory.GetComponents<HealthComponent>().size(), 2u );
	EXPECT_EQ( factory.GetComponents<ArmorComponent>().size(), 1u );
}

TEST( ComponentFactoryTests, GetComponentsIsEmptyForUnusedTypes )
{
	ComponentFactory factory;

	EXPECT_TRUE( factory.GetComponents<HealthComponent>().empty() );
}

TEST( ComponentFactoryTests, RegisterAndUnregisterIgnoreNull )
{
	ComponentFactory factory;

	factory.RegisterComponent( nullptr );
	factory.UnregisterComponent( nullptr );

	EXPECT_TRUE( factory.GetComponents<HealthComponent>().empty() );
}

TEST( ComponentFactoryTests, UnregisterRemovesOnlyThatComponent )
{
	ComponentFactory factory;

	shared<Component> first  = new_shared<HealthComponent>();
	shared<Component> second = new_shared<HealthComponent>();
	factory.RegisterComponent( first );
	factory.RegisterComponent( second );

	factory.UnregisterComponent( first );

	std::vector<shared<HealthComponent>> components = factory.GetComponents<HealthComponent>();
	ASSERT_EQ( components.size(), 1u );
	EXPECT_EQ( components[0], second );
}

TEST( ComponentFactoryTests, DestroyedComponentsAreNotReturned )
{
	ComponentFactory factory;

	shared<Component> kept      = new_shared<HealthComponent>();
	shared<Component> destroyed = new_shared<HealthComponent>();
	factory.RegisterComponent( kept );
	factory.RegisterComponent( destroyed );

	destroyed.reset();

	std::vector<shared<HealthComponent>> components = factory.GetComponents<HealthComponent>();
	ASSERT_EQ( components.size(), 1u );
	EXPECT_EQ( components[0], kept );
}

TEST( ComponentFactoryTests, ClearForgetsComponentsButNotCreators )
{
	ComponentFactory factory;

	shared<Component> component = new_shared<HealthComponent>();
	factory.RegisterComponent( component );

	factory.Clear();

	EXPECT_TRUE( factory.GetComponents<HealthComponent>().empty() );
	EXPECT_NE( factory.Create( "HealthComponent" ), nullptr );
}
