#pragma once

#include <math/transform.h>
#include <scene/component.h>
#include <system/rtti.h>

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

class Level;

class Entity
{
	RTTI_DECLARE_TYPE( Entity )
public:
	Entity() = default;
	Entity( u64 id, std::string_view entityName, Level* level );
	~Entity();

	void Update();

	void OnAddToLevel();
	void OnRemoveFromLevel();

	Level*           GetLevel() const { return m_Level; }
	u64              GetId() const { return m_Id; }
	std::string_view GetName() const { return m_Name; }
	Transform        GetTransform() const { return m_Transform; }
	Transform&       GetTransformRef() { return m_Transform; }

	template <typename T, typename... Args>
	shared<T> AddComponent( Args&&... args );

	template <typename T>
	bool RemoveComponent();

	template <typename T>
	T* GetComponent();

	template <typename T>
	const T* GetComponent() const;

	bool RemoveComponent( Component* component ) { return RemoveComponentInternal( component ); }

	const std::vector<shared<Component>>& GetComponents() const { return m_Components; }

	bool SerializeComponents( Json::Value& outComponents ) const;
	bool DeserializeComponents( const Json::Value& inComponents );

private:
	friend class Level;

	void AttachToLevel( Level* level );

	bool AddComponentInternal( const shared<Component>& component );
	bool RemoveComponentInternal( Component* component );
	void ClearComponents();

	u64                            m_Id = 0;
	std::string                    m_Name;
	Transform                      m_Transform;
	Level*                         m_Level = nullptr;
	std::vector<shared<Component>> m_Components;
};

template <typename T, typename... Args>
shared<T> Entity::AddComponent( Args&&... args )
{
	static_assert( std::is_base_of_v<Component, T>, "T must derive from Component" );

	shared<T> component = new_shared<T>( std::forward<Args>( args )... );
	if( !AddComponentInternal( component ) )
		return nullptr;

	return component;
}

template <typename T>
bool Entity::RemoveComponent()
{
	static_assert( std::is_base_of_v<Component, T>, "T must derive from Component" );

	if( T* component = GetComponent<T>() )
		return RemoveComponentInternal( component );

	return false;
}

template <typename T>
T* Entity::GetComponent()
{
	static_assert( std::is_base_of_v<Component, T>, "T must derive from Component" );

	for( const shared<Component>& component : m_Components )
	{
		if( component == nullptr )
			continue;

		if( T* castedComponent = dynamic_cast<T*>( component.get() ) )
			return castedComponent;
	}

	return nullptr;
}

template <typename T>
const T* Entity::GetComponent() const
{
	static_assert( std::is_base_of_v<Component, T>, "T must derive from Component" );

	for( const shared<Component>& component : m_Components )
	{
		if( component == nullptr )
			continue;

		if( const T* castedComponent = dynamic_cast<const T*>( component.get() ) )
			return castedComponent;
	}

	return nullptr;
}