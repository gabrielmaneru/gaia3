#include "gaia3_pch.h"
#include "entity.h"

#include <scene/component.h>
#include <scene/componentFactory.h>
#include <scene/level.h>

#include <algorithm>

RTTI_DEFINE_TYPE( Entity,
                  RTTI_PROPERTY( m_Name ),
                  RTTI_PROPERTY( m_Transform ) )

Entity::Entity( u64 id, std::string_view entityName, Level* level )
    : m_Id( id ), m_Name( entityName ), m_Transform( Transform::V_Identity )
{
	AttachToLevel( level );
}

Entity::~Entity()
{
	ClearComponents();
	OnRemoveFromLevel();
}

void Entity::Update()
{
	for( auto& pComp : m_Components )
	{
		pComp->Update();
	}
}

void Entity::OnAddToLevel()
{
}

void Entity::OnRemoveFromLevel()
{
}

bool Entity::SerializeComponents( Json::Value& outComponents ) const
{
	outComponents = Json::Value( Json::arrayValue );

	for( const shared<Component>& component : m_Components )
	{
		if( component == nullptr )
			continue;

		Json::Value componentJson;
		if( !rtti::SerializeObject( component.get(), component->GetTypeInfo(), componentJson ) )
			return false;

		Json::Value serializedComponent( Json::objectValue );
		serializedComponent["m_TypeName"] = component->GetTypeInfo().Name;
		serializedComponent["m_Data"]     = std::move( componentJson );
		outComponents.append( std::move( serializedComponent ) );
	}

	return true;
}

bool Entity::DeserializeComponents( const Json::Value& inComponents )
{
	if( !inComponents.isArray() )
	{
		GAIA_ERROR( "Entity {}: component list is not an array", m_Name );
		return false;
	}

	bool allComponentsRestored = true;
	ClearComponents();

	for( const Json::Value& serializedComponent : inComponents )
	{
		if( !serializedComponent.isObject() )
		{
			GAIA_ERROR( "Entity {}: component entry is not an object", m_Name );
			allComponentsRestored = false;
			continue;
		}

		const Json::Value& componentTypeJson = serializedComponent["m_TypeName"];
		const Json::Value& componentDataJson = serializedComponent["m_Data"];
		if( !componentTypeJson.isString() || !componentDataJson.isObject() )
		{
			GAIA_ERROR( "Entity {}: component entry has no valid type or data", m_Name );
			allComponentsRestored = false;
			continue;
		}

		shared<Component> component = m_Level != nullptr ? m_Level->GetComponentFactory().Create( componentTypeJson.asString() ) : nullptr;
		if( component == nullptr )
		{
			GAIA_ERROR( "Entity {}: unknown component type {}", m_Name, componentTypeJson.asString() );
			allComponentsRestored = false;
			continue;
		}

		if( !rtti::DeserializeObject( component.get(), component->GetTypeInfo(), componentDataJson ) )
		{
			GAIA_ERROR( "Entity {}: invalid data for component {}", m_Name, componentTypeJson.asString() );
			allComponentsRestored = false;
			continue;
		}

		if( !AddComponentInternal( component ) )
		{
			GAIA_ERROR( "Entity {}: failed to add component {}", m_Name, componentTypeJson.asString() );
			allComponentsRestored = false;
			continue;
		}
	}

	return allComponentsRestored;
}

void Entity::AttachToLevel( Level* level )
{
	GAIA_ASSERT( level != nullptr );
	GAIA_ASSERT( m_Level == nullptr );
	m_Level = level;
	OnAddToLevel();
}

bool Entity::AddComponentInternal( const shared<Component>& component )
{
	if( component == nullptr || component->GetOwner() != nullptr )
		return false;

	m_Components.push_back( component );
	component->SetOwner( this );
	component->OnAddComponent();

	if( m_Level != nullptr )
		m_Level->RegisterComponent( component );

	return true;
}

bool Entity::RemoveComponentInternal( Component* component )
{
	auto it = std::find_if( m_Components.begin(),
	                        m_Components.end(),
	                        [component]( const shared<Component>& current ) { return current.get() == component; } );
	if( it == m_Components.end() )
		return false;

	shared<Component> componentToRemove = *it;

	if( m_Level != nullptr )
		m_Level->UnregisterComponent( componentToRemove );

	componentToRemove->OnRemoveComponent();
	componentToRemove->SetOwner( nullptr );

	m_Components.erase( it );
	return true;
}

void Entity::ClearComponents()
{
	for( const shared<Component>& component : m_Components )
	{
		if( component == nullptr )
			continue;

		if( m_Level != nullptr )
			m_Level->UnregisterComponent( component );

		component->OnRemoveComponent();
		component->SetOwner( nullptr );
	}

	m_Components.clear();
}
