#pragma once

#include <scene/component.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

class ComponentFactory
{
public:
	using CreateFunc = std::function<shared<Component>()>;

	static bool Register( std::string_view typeName, CreateFunc createFunc );

	shared<Component> Create( std::string_view typeName ) const;

	void RegisterComponent( const shared<Component>& component );
	void UnregisterComponent( const shared<Component>& component );
	void Clear();

	template <typename T>
	static bool RegisterType();

	template <typename T>
	std::vector<shared<T>> GetComponents();

private:
	static std::unordered_map<std::string, CreateFunc>& GetCreators();

	using WeakComponent = std::weak_ptr<Component>;
	std::unordered_map<u64, std::vector<WeakComponent>> m_ComponentsByType;
};

#define RTTI_DECLARE_COMP( TypeName, BaseType )                 \
public:                                                         \
	using Super = BaseType;                                     \
	static const ::rtti::TypeInfo& StaticTypeInfo();            \
	const ::rtti::TypeInfo&        GetTypeInfo() const override \
	{                                                           \
		return TypeName::StaticTypeInfo();                      \
	}                                                           \
                                                                \
private:

#define RTTI_DEFINE_COMP( TypeName, ... )                                             \
	RTTI_DEFINE_TYPE( TypeName, __VA_ARGS__ )                                         \
	namespace                                                                         \
	{                                                                                 \
	const bool g_##TypeName##Registered = ComponentFactory::RegisterType<TypeName>(); \
	}

template <typename T>
static bool ComponentFactory::RegisterType()
{
	static_assert( std::is_base_of_v<Component, T>, "T must derive from Component" );
	return Register( T::StaticTypeInfo().Name, []() -> shared<Component> { return new_shared<T>(); } );
}

template <typename T>
std::vector<shared<T>> ComponentFactory::GetComponents()
{
	static_assert( std::is_base_of_v<Component, T>, "T must derive from Component" );

	std::vector<shared<T>> result;
	auto                   it = m_ComponentsByType.find( T::StaticTypeInfo().TypeId );
	if( it == m_ComponentsByType.end() )
		return result;

	auto& components = it->second;
	for( auto iter = components.begin(); iter != components.end(); )
	{
		shared<Component> component = iter->lock();
		if( component == nullptr )
		{
			iter = components.erase( iter );
			continue;
		}

		if( shared<T> typed = std::dynamic_pointer_cast<T>( component ) )
			result.push_back( typed );

		++iter;
	}

	return result;
}