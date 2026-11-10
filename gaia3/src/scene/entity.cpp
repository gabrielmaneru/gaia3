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
}

void Entity::Update()
{
	for( auto& pComp : m_Components )
	{
		pComp->Update();
	}
}

void Entity::SetTransform( const Transform& newTransform )
{
	m_Transform = newTransform;
	NotifyTransformChanged();
}

void Entity::SetPosition( const Vec3& newPosition )
{
	m_Transform.position = newPosition;
	NotifyTransformChanged();
}

void Entity::SetRotation( const Quat& newRotation )
{
	m_Transform.rotation = newRotation;
	NotifyTransformChanged();
}

void Entity::SetScale( const Vec3& newScale )
{
	m_Transform.scale = newScale;
	NotifyTransformChanged();
}

Component* Entity::AddComponentByName( std::string_view typeName )
{
	shared<Component> component = m_Level != nullptr ? m_Level->GetComponentFactory().Create( typeName ) : nullptr;
	if( component == nullptr || !AddComponentInternal( component ) )
	{
		GAIA_ERROR( "Entity {}: failed to add component {}", m_Name, typeName );
		return nullptr;
	}

	return component.get();
}

bool Entity::HasComponent( std::string_view typeName ) const
{
	return std::any_of( m_Components.begin(), m_Components.end(), [typeName]( const shared<Component>& component ) {
		return component != nullptr && typeName == component->GetTypeInfo().Name;
	} );
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

		if( HasComponent( componentTypeJson.asString() ) )
		{
			GAIA_WARN( "Entity {}: ignoring a duplicate {}, keeping the first one", m_Name, componentTypeJson.asString() );
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
}

bool Entity::AddComponentInternal( const shared<Component>& component )
{
	if( component == nullptr || component->HasOwner() )
		return false;

	if( HasComponent( component->GetTypeInfo().Name ) )
	{
		GAIA_ERROR( "Entity {}: already has a {}", m_Name, component->GetTypeInfo().Name );
		return false;
	}

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

void Entity::NotifyTransformChanged()
{
	for( auto& pComp : m_Components )
	{
		pComp->OnTransformChanged();
	}
}
