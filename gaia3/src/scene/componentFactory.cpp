#include "gaia3_pch.h"
#include "componentFactory.h"

#include <unordered_map>

std::unordered_map<std::string, ComponentFactory::CreateFunc>& ComponentFactory::GetCreators()
{
	static std::unordered_map<std::string, CreateFunc> creators;
	return creators;
}

bool ComponentFactory::Register( std::string_view typeName, CreateFunc createFunc )
{
	auto& creators = GetCreators();
	return creators.insert_or_assign( std::string( typeName ), std::move( createFunc ) ).second;
}

std::vector<std::string> ComponentFactory::GetRegisteredTypeNames()
{
	std::vector<std::string> typeNames;
	for( const auto& [typeName, createFunc] : GetCreators() )
	{
		typeNames.push_back( typeName );
	}

	std::sort( typeNames.begin(), typeNames.end() );
	return typeNames;
}

shared<Component> ComponentFactory::Create( std::string_view typeName ) const
{
	auto& creators = GetCreators();
	auto  it       = creators.find( std::string( typeName ) );
	if( it == creators.end() )
		return nullptr;

	return it->second != nullptr ? it->second() : nullptr;
}

void ComponentFactory::RegisterComponent( const shared<Component>& component )
{
	if( component == nullptr )
		return;

	m_ComponentsByType[component->GetTypeInfo().TypeId].push_back( component );
}

void ComponentFactory::UnregisterComponent( const shared<Component>& component )
{
	if( component == nullptr )
		return;

	auto it = m_ComponentsByType.find( component->GetTypeInfo().TypeId );
	if( it == m_ComponentsByType.end() )
		return;

	auto& components = it->second;
	for( auto iter = components.begin(); iter != components.end(); )
	{
		shared<Component> existing = iter->lock();
		if( existing == nullptr || existing.get() == component.get() )
		{
			iter = components.erase( iter );
			continue;
		}

		++iter;
	}

	if( components.empty() )
		m_ComponentsByType.erase( it );
}

void ComponentFactory::Clear()
{
	m_ComponentsByType.clear();
}
