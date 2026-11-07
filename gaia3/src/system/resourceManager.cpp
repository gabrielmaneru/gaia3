#include "gaia3_pch.h"
#include "resourceManager.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace
{
std::string ToLower( std::string value )
{
	std::transform( value.begin(), value.end(), value.begin(), []( unsigned char c ) {
		return static_cast<char>( std::tolower( c ) );
	} );
	return value;
}

std::string BuildResourceKey( const std::filesystem::path& rootPath, const std::filesystem::path& filePath )
{
	std::error_code       ec;
	std::filesystem::path relativePath = std::filesystem::relative( filePath, rootPath, ec );
	if( ec )
		relativePath = filePath.filename();

	relativePath.replace_extension();
	return relativePath.generic_string();
}
} // namespace

bool ResourceManager::Init( std::string_view commonPath, std::string_view appPath )
{
	Shutdown();
	m_CommonPath = commonPath;
	m_AppPath    = appPath;

	auto scanPath = [this]( std::string_view rootPath ) {
		if( rootPath.empty() )
			return;

		std::filesystem::path root( rootPath );
		if( !std::filesystem::exists( root ) || !std::filesystem::is_directory( root ) )
		{
			GAIA_WARN( "ResourceManager: folder {} not found, skipping it", rootPath );
			return;
		}

		u64 resourceCount = 0;
		for( const std::filesystem::directory_entry& entry :
		     std::filesystem::recursive_directory_iterator( root, std::filesystem::directory_options::skip_permission_denied ) )
		{
			if( !entry.is_regular_file() )
				continue;

			const std::filesystem::path&    filePath      = entry.path();
			std::string                     fileExtension = ToLower( filePath.extension().string() );
			const Resource::TypeDescriptor* descriptor    = Resource::FindResourceTypeByExtension( fileExtension );
			if( descriptor == nullptr )
				continue;

			std::string resourceKey = BuildResourceKey( root, filePath );
			if( resourceKey.empty() )
				resourceKey = filePath.stem().string();

			RegisterResource( descriptor->TypeId, resourceKey, descriptor->Create( filePath.string() ) );
			++resourceCount;
		}

		GAIA_INFO( "ResourceManager: found {} resources in {}", resourceCount, rootPath );
	};

	scanPath( commonPath );
	scanPath( appPath );

	return true;
}

void ResourceManager::Shutdown()
{
	for( auto& [_, resourceMap] : m_ResourceMaps )
	{
		for( auto& [__, resource] : resourceMap )
		{
			if( resource != nullptr )
				resource->Unload();
		}
	}

	m_ResourceMaps.clear();
}

bool ResourceManager::SaveResourceAsType( u64 typeId, const shared<Resource>& resource, std::string_view fileName )
{
	if( resource == nullptr || fileName.empty() || m_AppPath.empty() )
	{
		GAIA_ERROR( "ResourceManager: cannot save a resource without a resource, file name and app path" );
		return false;
	}

	std::filesystem::path outputPath = std::filesystem::path( m_AppPath ) / std::string( fileName );
	if( !resource->SaveAs( outputPath.string() ) )
		return false;

	RemoveResourceEntries( typeId, resource );

	std::string resourceKey = BuildResourceKey( std::filesystem::path( m_AppPath ), outputPath );
	if( resourceKey.empty() )
		resourceKey = outputPath.stem().string();

	RegisterResource( typeId, resourceKey, resource );
	return true;
}

shared<Resource> ResourceManager::FindResourceByTypeId( u64 typeId, std::string_view resourceId, bool forceLoad )
{
	const std::string resourceIdString( resourceId );

	auto typeIt = m_ResourceMaps.find( typeId );
	if( typeIt == m_ResourceMaps.end() )
		return {};

	const ResourceMap& resourceMap    = typeIt->second;
	auto               returnResource = [forceLoad]( const shared<Resource>& resource ) -> shared<Resource> {
		if( resource == nullptr )
			return {};

		if( forceLoad && !resource->Load() )
			return {};

		return resource;
	};

	auto tryFindKey = [&]( const std::string& key ) -> shared<Resource> {
		auto resourceIt = resourceMap.find( key );
		if( resourceIt == resourceMap.end() )
			return {};

		return returnResource( resourceIt->second );
	};

	if( shared<Resource> resource = tryFindKey( resourceIdString ) )
		return resource;

	std::filesystem::path requestedPath( resourceIdString );
	const std::string     requestedGeneric = requestedPath.generic_string();
	if( requestedGeneric != resourceIdString )
	{
		if( shared<Resource> resource = tryFindKey( requestedGeneric ) )
			return resource;
	}

	std::filesystem::path noExtPath = requestedPath;
	noExtPath.replace_extension();
	const std::string requestedKeyNoExt = noExtPath.generic_string();
	if( !requestedKeyNoExt.empty() )
	{
		if( shared<Resource> resource = tryFindKey( requestedKeyNoExt ) )
			return resource;
	}

	const std::string requestedNameNoExt = requestedPath.stem().string();
	for( const auto& [_, typedResource] : resourceMap )
	{
		if( typedResource == nullptr )
			continue;

		if( typedResource->GetName() == resourceIdString || ( !requestedNameNoExt.empty() && typedResource->GetName() == requestedNameNoExt ) )
		{
			return returnResource( typedResource );
		}
	}

	return {};
}

void ResourceManager::RegisterResource( u64 typeId, const std::string& resourceKey, shared<Resource> resource )
{
	if( resource == nullptr || resourceKey.empty() )
		return;

	ResourceMap& resourceMap = m_ResourceMaps[typeId];
	resourceMap[resourceKey] = resource;
}

void ResourceManager::RemoveResourceEntries( u64 typeId, const shared<Resource>& resource )
{
	auto mapIt = m_ResourceMaps.find( typeId );
	if( mapIt == m_ResourceMaps.end() || resource == nullptr )
		return;

	ResourceMap& resourceMap = mapIt->second;
	for( auto it = resourceMap.begin(); it != resourceMap.end(); )
	{
		if( it->second == resource )
			it = resourceMap.erase( it );
		else
			++it;
	}
}
