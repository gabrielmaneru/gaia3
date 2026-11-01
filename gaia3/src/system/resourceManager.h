#pragma once

#include <core/types.h>
#include <system/resource.h>
#include <system/type_id.h>

#include <string>
#include <string_view>
#include <unordered_map>

class ResourceManager
{
public:
	bool Init( std::string_view commonPath, std::string_view appPath );
	void Shutdown();

	template <typename TResource>
	shared<TResource> FindResource( std::string_view resourceId, bool forceLoad = false );
	template <typename TResource>
	bool SaveResourceAs( const shared<TResource>& resource, std::string_view fileName );

private:
	using ResourceMap = std::unordered_map<std::string, shared<Resource>>;

	shared<Resource> FindResourceByTypeId( u64 typeId, std::string_view resourceId, bool forceLoad );
	bool             SaveResourceAsType( u64 typeId, const shared<Resource>& resource, std::string_view fileName );

	void RegisterResource( u64 typeId, const std::string& resourceKey, shared<Resource> resource );
	void RemoveResourceEntries( u64 typeId, const shared<Resource>& resource );

	std::unordered_map<u64, ResourceMap> m_ResourceMaps;
	std::string                          m_CommonPath;
	std::string                          m_AppPath;
};

template <typename TResource>
shared<TResource> ResourceManager::FindResource( std::string_view resourceId, bool forceLoad )
{
	shared<Resource> resource = FindResourceByTypeId( GetTypeId<TResource>(), resourceId, forceLoad );
	return resource != nullptr ? std::static_pointer_cast<TResource>( resource ) : nullptr;
}

template <typename TResource>
bool ResourceManager::SaveResourceAs( const shared<TResource>& resource, std::string_view fileName )
{
	return SaveResourceAsType( GetTypeId<TResource>(), resource, fileName );
}
