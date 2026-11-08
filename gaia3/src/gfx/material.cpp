#include "gaia3_pch.h"
#include "material.h"

#include <core/app.h>
#include <gfx/shader.h>
#include <gfx/texture.h>

#include <algorithm>
#include <fstream>
#include <sstream>

RTTI_DEFINE_RESOURCE( Material,
                      RTTI_RESOURCE_FOLDER( "materials" ),
                      RTTI_RESOURCE_EXTENSIONS( ".mtl" ) )

namespace
{
constexpr u32 kDiffuseTextureUnit = 0;

bool ParseVec3( std::istringstream& stream, Vec3& outValue )
{
	return static_cast<bool>( stream >> outValue.x >> outValue.y >> outValue.z );
}
} // namespace

void MaterialEntry::Bind( const DrawContext& context ) const
{
	if( shader == nullptr )
		return;

	shader->Use();
	shader->SetMatrix44( "u_Model", context.model );
	shader->SetMatrix44( "u_View", context.view );
	shader->SetMatrix44( "u_Projection", context.projection );

	shader->SetVec3( "u_Color", data.diffuseColor );
	shader->SetInt( "u_UseTexture", diffuseTexture != nullptr ? 1 : 0 );
	if( diffuseTexture != nullptr )
	{
		diffuseTexture->Bind( kDiffuseTextureUnit );
		shader->SetInt( "u_Texture", kDiffuseTextureUnit );
	}
}

Material::Material( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

bool Material::ReadMtl( std::string_view path, std::vector<MaterialData>& outMaterials )
{
	outMaterials.clear();

	std::ifstream file{ std::string( path ) };
	if( !file.is_open() )
	{
		GAIA_ERROR( "Material: failed to open {}", path );
		return false;
	}

	// Note: Lines before the first newmtl start an unnamed material
	auto currentMaterial = [&outMaterials]() -> MaterialData& {
		if( outMaterials.empty() )
		{
			outMaterials.emplace_back();
		}
		return outMaterials.back();
	};

	u64         lineNumber = 0;
	std::string line;
	while( std::getline( file, line ) )
	{
		++lineNumber;

		std::istringstream lineStream( line );
		std::string        keyword;
		lineStream >> keyword;

		if( keyword == "newmtl" )
		{
			MaterialData material;
			lineStream >> material.name;

			const bool bRepeated = std::any_of( outMaterials.begin(), outMaterials.end(), [&material]( const MaterialData& existing ) {
				return existing.name == material.name;
			} );
			if( bRepeated )
			{
				GAIA_WARN( "Material: {} defines {} twice, the first one is used", path, material.name );
			}

			outMaterials.push_back( material );
		}
		else if( keyword == "Kd" )
		{
			if( !ParseVec3( lineStream, currentMaterial().diffuseColor ) )
			{
				GAIA_ERROR( "Material: invalid colour at {}:{}", path, lineNumber );
				return false;
			}
		}
		else if( keyword == "map_Kd" )
		{
			// Note: Options like "-s 1 1 1" come before the file name, so the file is the last word
			std::string word;
			while( lineStream >> word )
			{
				currentMaterial().diffuseTexture = word;
			}
		}
		else if( keyword == "shader" )
		{
			lineStream >> currentMaterial().shader;
		}
	}

	currentMaterial();
	return true;
}

shared<Material> Material::FindOrError( std::string_view libraryName, std::string_view requester )
{
	ResourceManager& manager = App::GetSession().GetResourceManager();

	if( shared<Material> library = manager.FindResource<Material>( libraryName, true ) )
		return library;

	GAIA_WARN( "{}: material {} is not available, using {}", requester, libraryName, kErrorName );

	shared<Material> errorLibrary = manager.FindResource<Material>( kErrorName, true );
	if( errorLibrary == nullptr )
	{
		GAIA_ERROR( "{}: the {} material is not available either, so nothing draws in its place", requester, kErrorName );
	}

	return errorLibrary;
}

const MaterialEntry* Material::FindErrorEntry( std::string_view requester )
{
	shared<Material>     errorLibrary = App::GetSession().GetResourceManager().FindResource<Material>( kErrorName, true );
	const MaterialEntry* errorEntry   = errorLibrary != nullptr ? errorLibrary->FindEntry( "" ) : nullptr;
	if( errorEntry == nullptr )
	{
		GAIA_ERROR( "{}: the {} material is not available, so nothing draws in its place", requester, kErrorName );
	}

	return errorEntry;
}

bool Material::LoadInternal()
{
	std::vector<MaterialData> materials;
	if( !ReadMtl( GetPath(), materials ) )
		return false;

	ResourceManager& manager = App::GetSession().GetResourceManager();

	for( MaterialData& data : materials )
	{
		MaterialEntry entry;
		entry.shader = manager.FindResource<Shader>( data.shader, true );
		if( entry.shader == nullptr )
		{
			GAIA_ERROR( "Material {}: shader {} of {} is not available", GetName(), data.shader, data.name );
			m_Entries.clear();
			return false;
		}

		if( !data.diffuseTexture.empty() )
		{
			entry.diffuseTexture = Texture::FindOrError( data.diffuseTexture, "Material " + std::string( GetName() ) );
		}

		entry.data = std::move( data );
		m_Entries.push_back( std::move( entry ) );
	}

	return true;
}

bool Material::UnloadInternal()
{
	m_Entries.clear();
	return true;
}

const MaterialEntry* Material::FindEntry( std::string_view materialName ) const
{
	if( m_Entries.empty() )
		return nullptr;

	if( materialName.empty() )
		return &m_Entries.front();

	for( const MaterialEntry& entry : m_Entries )
	{
		if( entry.data.name == materialName )
			return &entry;
	}

	return nullptr;
}
