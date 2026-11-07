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

std::string RemoveTypeFolder( const std::string& resourceKey, const std::string& folder )
{
	const std::string folderPrefix = folder + "/";
	return resourceKey.starts_with( folderPrefix ) ? resourceKey.substr( folderPrefix.size() ) : resourceKey;
}

// Note: Files inside the folder of their type are named without it, any other file keeps its whole relative path
std::string BuildResourceKey( const std::filesystem::path& rootPath, const std::filesystem::path& filePath, const std::string& folder )
{
	std::error_code       ec;
	std::filesystem::path relativePath = std::filesystem::relative( filePath, rootPath, ec );
	if( ec )
		relativePath = filePath.filename();

	relativePath.replace_extension();
	return RemoveTypeFolder( relativePath.generic_string(), folder );
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

			std::string resourceKey = BuildResourceKey( root, filePath, descriptor->Folder );
			if( resourceKey.empty() )
				resourceKey = filePath.stem().string();

			if( !WinsNameClash( *descriptor, resourceKey, filePath.string() ) )
				continue;

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
	const Resource::TypeDescriptor* descriptor = Resource::FindResourceTypeById( typeId );
	if( resource == nullptr || descriptor == nullptr || fileName.empty() || m_AppPath.empty() )
	{
		GAIA_ERROR( "ResourceManager: cannot save a resource without a resource, registered type, file name and app path" );
		return false;
	}

	const std::string     previousPath( resource->GetPath() );
	std::filesystem::path outputPath = std::filesystem::path( m_AppPath ) / descriptor->Folder / std::string( fileName );

	std::error_code ec;
	std::filesystem::create_directories( outputPath.parent_path(), ec );

	if( !resource->SaveAs( outputPath.string() ) )
		return false;

	ReleaseResourceEntries( typeId, resource, previousPath );

	std::string resourceKey = BuildResourceKey( std::filesystem::path( m_AppPath ), outputPath, descriptor->Folder );
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

		// Note: The folder of the type can still be written, older levels reference resources like "models/cube"
		const Resource::TypeDescriptor* descriptor = Resource::FindResourceTypeById( typeId );
		if( descriptor != nullptr )
		{
			if( shared<Resource> resource = tryFindKey( RemoveTypeFolder( requestedKeyNoExt, descriptor->Folder ) ) )
				return resource;
		}
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

std::vector<std::string> ResourceManager::GetResourceNamesByTypeId( u64 typeId ) const
{
	std::vector<std::string> names;

	auto mapIt = m_ResourceMaps.find( typeId );
	if( mapIt == m_ResourceMaps.end() )
		return names;

	for( const auto& [resourceKey, resource] : mapIt->second )
	{
		names.push_back( resourceKey );
	}

	std::sort( names.begin(), names.end() );
	return names;
}

bool ResourceManager::IsInTypeFolder( std::string_view filePath, const std::string& folder ) const
{
	const std::string folderPrefix = folder + "/";
	for( const std::string& root : { m_CommonPath, m_AppPath } )
	{
		if( !root.empty() && std::filesystem::path( filePath ).lexically_relative( root ).generic_string().starts_with( folderPrefix ) )
			return true;
	}

	return false;
}

bool ResourceManager::WinsNameClash( const Resource::TypeDescriptor& descriptor, const std::string& resourceKey, const std::string& filePath ) const
{
	auto mapIt = m_ResourceMaps.find( descriptor.TypeId );
	if( mapIt == m_ResourceMaps.end() )
		return true;

	auto resourceIt = mapIt->second.find( resourceKey );
	if( resourceIt == mapIt->second.end() )
		return true;

	// Note: The type folder takes precedence whatever the scan order is, and otherwise the app overrides common
	const std::string existingPath( resourceIt->second->GetPath() );
	const bool        bExistingInFolder = IsInTypeFolder( existingPath, descriptor.Folder );
	const bool        bNewInFolder      = IsInTypeFolder( filePath, descriptor.Folder );
	if( bExistingInFolder == bNewInFolder )
		return true;

	GAIA_WARN( "ResourceManager: ignoring {}, {} uses the name {} from the {} folder",
	           bNewInFolder ? existingPath : filePath,
	           bNewInFolder ? filePath : existingPath,
	           resourceKey,
	           descriptor.Folder );
	return bNewInFolder;
}

void ResourceManager::RegisterResource( u64 typeId, const std::string& resourceKey, shared<Resource> resource )
{
	if( resource == nullptr || resourceKey.empty() )
		return;

	ResourceMap& resourceMap = m_ResourceMaps[typeId];
	resourceMap[resourceKey] = resource;
}

void ResourceManager::ReleaseResourceEntries( u64 typeId, const shared<Resource>& resource, std::string_view previousPath )
{
	auto mapIt = m_ResourceMaps.find( typeId );
	if( mapIt == m_ResourceMaps.end() || resource == nullptr )
		return;

	const std::string               extension  = std::filesystem::path( previousPath ).extension().string();
	const Resource::TypeDescriptor* descriptor = Resource::FindResourceTypeByExtension( extension );

	// Note: The previous file is still on disk, so its name stays available as a separate unloaded resource
	ResourceMap& resourceMap = mapIt->second;
	for( auto it = resourceMap.begin(); it != resourceMap.end(); )
	{
		if( it->second != resource )
		{
			++it;
		}
		else if( descriptor != nullptr )
		{
			it->second = descriptor->Create( previousPath );
			++it;
		}
		else
		{
			it = resourceMap.erase( it );
		}
	}
}
