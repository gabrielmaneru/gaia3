#include "testHelpers.h"

#include "testComponents.h"

#include <scene/level.h>

TEST( EntityTests, AddComponentSetsOwnerAndNotifiesTheComponent )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> component = entity->AddComponent<HealthComponent>();

	ASSERT_NE( component, nullptr );
	EXPECT_EQ( component->GetOwner(), entity );
	EXPECT_EQ( component->addCount, 1 );
}

TEST( EntityTests, AddComponentForwardsConstructorArguments )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> component = entity->AddComponent<HealthComponent>( 7 );

	EXPECT_EQ( component->m_Health, 7 );
}

TEST( EntityTests, AddComponentRegistersWithTheLevel )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	entity->AddComponent<HealthComponent>();

	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 1u );
}

TEST( EntityTests, GetComponentFindsByType )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> health = entity->AddComponent<HealthComponent>();
	shared<ArmorComponent>  armor  = entity->AddComponent<ArmorComponent>();

	const Entity* constEntity = entity;
	EXPECT_EQ( entity->GetComponent<HealthComponent>(), health.get() );
	EXPECT_EQ( constEntity->GetComponent<ArmorComponent>(), armor.get() );
}

TEST( EntityTests, GetComponentIsNullWhenMissing )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	EXPECT_EQ( entity->GetComponent<HealthComponent>(), nullptr );
}

TEST( EntityTests, RemoveComponentByTypeClearsOwnerAndNotifiesTheComponent )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> component = entity->AddComponent<HealthComponent>();

	EXPECT_TRUE( entity->RemoveComponent<HealthComponent>() );

	EXPECT_EQ( component->GetOwner(), nullptr );
	EXPECT_EQ( component->removeCount, 1 );
	EXPECT_EQ( entity->GetComponent<HealthComponent>(), nullptr );
	EXPECT_TRUE( level.GetComponents<HealthComponent>().empty() );
}

TEST( EntityTests, RemoveComponentByPointerRemovesOnlyThatComponent )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> health = entity->AddComponent<HealthComponent>();
	shared<ArmorComponent>  armor  = entity->AddComponent<ArmorComponent>();

	EXPECT_TRUE( entity->RemoveComponent( health.get() ) );

	EXPECT_EQ( entity->GetComponents().size(), 1u );
	EXPECT_EQ( entity->GetComponent<ArmorComponent>(), armor.get() );
}

TEST( EntityTests, ASecondComponentOfTheSameTypeIsRejected )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> first = entity->AddComponent<HealthComponent>( 1 );

	LogCapture log;
	EXPECT_EQ( entity->AddComponent<HealthComponent>( 2 ), nullptr );

	EXPECT_TRUE( log.Contains( "already has a HealthComponent" ) );
	EXPECT_EQ( entity->GetComponents().size(), 1u );
	EXPECT_EQ( entity->GetComponent<HealthComponent>(), first.get() );
	EXPECT_EQ( first->m_Health, 1 );
	EXPECT_EQ( first->addCount, 1 );
	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 1u );
}

TEST( EntityTests, ASecondComponentByNameIsRejected )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	ASSERT_NE( entity->AddComponentByName( "HealthComponent" ), nullptr );

	EXPECT_EQ( entity->AddComponentByName( "HealthComponent" ), nullptr );
	EXPECT_EQ( entity->GetComponents().size(), 1u );
}

TEST( EntityTests, AComponentTypeCanBeAddedAgainAfterRemovingIt )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	entity->AddComponent<HealthComponent>( 1 );
	ASSERT_TRUE( entity->RemoveComponent<HealthComponent>() );

	shared<HealthComponent> replacement = entity->AddComponent<HealthComponent>( 2 );

	ASSERT_NE( replacement, nullptr );
	EXPECT_EQ( entity->GetComponent<HealthComponent>(), replacement.get() );
}

TEST( EntityTests, EachEntityCanHoldItsOwnComponentOfAType )
{
	Level level;

	EXPECT_NE( level.CreateEntity( "First" )->AddComponent<HealthComponent>(), nullptr );
	EXPECT_NE( level.CreateEntity( "Second" )->AddComponent<HealthComponent>(), nullptr );
	EXPECT_EQ( level.GetComponents<HealthComponent>().size(), 2u );
}

TEST( EntityTests, HasComponentChecksTheTypeName )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	EXPECT_FALSE( entity->HasComponent( "HealthComponent" ) );

	entity->AddComponent<HealthComponent>();

	EXPECT_TRUE( entity->HasComponent( "HealthComponent" ) );
	EXPECT_FALSE( entity->HasComponent( "ArmorComponent" ) );

	entity->RemoveComponent<HealthComponent>();

	EXPECT_FALSE( entity->HasComponent( "HealthComponent" ) );
}

TEST( EntityTests, RemovingAMissingComponentFails )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	HealthComponent notAttached;

	EXPECT_FALSE( entity->RemoveComponent<HealthComponent>() );
	EXPECT_FALSE( entity->RemoveComponent( &notAttached ) );
}

TEST( EntityTests, UpdateReachesEveryComponent )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> health = entity->AddComponent<HealthComponent>();
	shared<ArmorComponent>  armor  = entity->AddComponent<ArmorComponent>();

	entity->Update();

	EXPECT_EQ( health->updateCount, 1 );
	EXPECT_EQ( armor->updateCount, 1 );
}

TEST( EntityTests, DestroyingAnEntityRemovesItsComponents )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> component = entity->AddComponent<HealthComponent>();

	ASSERT_TRUE( level.RemoveEntity( entity->GetId() ) );

	EXPECT_EQ( component->removeCount, 1 );
	EXPECT_EQ( component->GetOwner(), nullptr );
	EXPECT_TRUE( level.GetComponents<HealthComponent>().empty() );
}

TEST( EntityTests, SerializeComponentsWritesTypeNameAndData )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );
	entity->AddComponent<HealthComponent>( 42 );

	Json::Value json;
	ASSERT_TRUE( entity->SerializeComponents( json ) );

	ASSERT_TRUE( json.isArray() );
	ASSERT_EQ( json.size(), 1u );
	EXPECT_EQ( json[0]["m_TypeName"].asString(), "HealthComponent" );
	EXPECT_EQ( json[0]["m_Data"]["m_Health"].asInt(), 42 );
}

TEST( EntityTests, ComponentsRoundTripThroughJson )
{
	Level   sourceLevel;
	Entity* source = sourceLevel.CreateEntity( "Source" );
	source->AddComponent<HealthComponent>( 42 );
	source->AddComponent<ArmorComponent>();

	Json::Value json;
	ASSERT_TRUE( source->SerializeComponents( json ) );

	Level   targetLevel;
	Entity* target = targetLevel.CreateEntity( "Target" );
	ASSERT_TRUE( target->DeserializeComponents( json ) );

	HealthComponent* health = target->GetComponent<HealthComponent>();
	ASSERT_NE( health, nullptr );
	EXPECT_EQ( health->m_Health, 42 );
	EXPECT_EQ( health->GetOwner(), target );
	EXPECT_EQ( health->addCount, 1 );
	EXPECT_NE( target->GetComponent<ArmorComponent>(), nullptr );
	EXPECT_EQ( targetLevel.GetComponents<HealthComponent>().size(), 1u );
}

TEST( EntityTests, DeserializeComponentsReplacesExistingComponents )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	shared<HealthComponent> existing = entity->AddComponent<HealthComponent>( 1 );

	Json::Value empty( Json::arrayValue );
	ASSERT_TRUE( entity->DeserializeComponents( empty ) );

	EXPECT_TRUE( entity->GetComponents().empty() );
	EXPECT_EQ( existing->removeCount, 1 );
}

TEST( EntityTests, DeserializeComponentsRejectsNonArrays )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );

	EXPECT_FALSE( entity->DeserializeComponents( Json::Value( Json::objectValue ) ) );
}

TEST( EntityTests, DeserializeComponentsKeepsTheFirstOfADuplicatedType )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );
	entity->AddComponent<HealthComponent>( 1 );

	Json::Value first;
	ASSERT_TRUE( entity->SerializeComponents( first ) );

	Json::Value second              = first;
	second[0]["m_Data"]["m_Health"] = 2;

	Json::Value json( Json::arrayValue );
	json.append( first[0] );
	json.append( second[0] );

	LogCapture log;
	EXPECT_TRUE( entity->DeserializeComponents( json ) );

	EXPECT_TRUE( log.Contains( "ignoring a duplicate HealthComponent" ) );
	ASSERT_EQ( entity->GetComponents().size(), 1u );
	EXPECT_EQ( entity->GetComponent<HealthComponent>()->m_Health, 1 );
}

TEST( EntityTests, DeserializeComponentsReportsBadEntriesButKeepsGoodOnes )
{
	Level   level;
	Entity* entity = level.CreateEntity( "Entity" );
	entity->AddComponent<HealthComponent>( 42 );

	Json::Value good;
	ASSERT_TRUE( entity->SerializeComponents( good ) );

	Json::Value unknownType( Json::objectValue );
	unknownType["m_TypeName"] = "NoSuchComponent";
	unknownType["m_Data"]     = Json::Value( Json::objectValue );

	Json::Value json( Json::arrayValue );
	json.append( 5 );
	json.append( unknownType );
	json.append( good[0] );

	EXPECT_FALSE( entity->DeserializeComponents( json ) );

	ASSERT_EQ( entity->GetComponents().size(), 1u );
	EXPECT_EQ( entity->GetComponent<HealthComponent>()->m_Health, 42 );
}
