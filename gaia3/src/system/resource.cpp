#include "gaia3_pch.h"
#include "resource.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

Resource::Resource( std::string_view path, ResourceState state )
    : m_State( state )
{
	SetPath( path );
}

void Resource::SetPath( std::string_view path )
{
	m_Path = path;

	std::filesystem::path filePath( path );
	m_Name = filePath.stem().string();
	if( m_Name.empty() )
	{
		m_Name = filePath.filename().string();
	}
}

bool Resource::Load()
{
	if( m_State == ResourceState::Loaded )
		return true;

	if( m_State == ResourceState::NotLoaded )
	{
		m_State = ResourceState::Loading;
		if( LoadInternal() )
		{
			m_State = ResourceState::Loaded;
			GAIA_INFO( "Loaded resource {}", m_Path );
			return true;
		}
		else
		{
			m_State = ResourceState::NotLoaded;
		}
	}

	GAIA_ERROR( "Failed to load resource {}", m_Path );
	return false;
}

bool Resource::Unload()
{

	if( m_State == ResourceState::Loaded )
	{
		if( UnloadInternal() )
		{
			m_State = ResourceState::NotLoaded;
			return true;
		}

		GAIA_ERROR( "Failed to unload resource {}", m_Path );
	}
	return false;
}

bool Resource::SaveAs( std::string_view path )
{
	if( SaveAsInternal( path ) )
	{
		SetPath( path );
		m_State = ResourceState::Loaded;
		GAIA_INFO( "Saved resource {}", m_Path );
		return true;
	}

	GAIA_ERROR( "Failed to save resource as {}", path );
	return false;
}

bool Resource::RegisterResourceType( const TypeDescriptor& descriptor )
{
	if( !descriptor.IsValid() )
		return false;

	TypeDescriptor normalizedDescriptor = descriptor;
	for( std::string& extension : normalizedDescriptor.Extensions )
	{
		std::transform( extension.begin(), extension.end(), extension.begin(), []( unsigned char c ) {
			return static_cast<char>( std::tolower( c ) );
		} );
	}

	GetTypeDescriptorsRef().push_back( std::move( normalizedDescriptor ) );
	return true;
}

const Resource::TypeDescriptor* Resource::FindResourceTypeByExtension( std::string_view extension )
{
	std::string normalizedExtension( extension );
	std::transform( normalizedExtension.begin(), normalizedExtension.end(), normalizedExtension.begin(), []( unsigned char c ) {
		return static_cast<char>( std::tolower( c ) );
	} );

	const std::vector<TypeDescriptor>& typeDescriptors = GetTypeDescriptorsRef();
	for( const TypeDescriptor& descriptor : typeDescriptors )
	{
		for( std::string_view supportedExtension : descriptor.Extensions )
		{
			if( supportedExtension == normalizedExtension )
				return &descriptor;
		}
	}

	return nullptr;
}

std::vector<Resource::TypeDescriptor>& Resource::GetTypeDescriptorsRef()
{
	static std::vector<Resource::TypeDescriptor> typeDescriptors;
	return typeDescriptors;
}