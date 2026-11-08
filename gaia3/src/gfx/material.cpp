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

Material::Material( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

std::vector<std::string> Material::ListEntries( std::string_view path )
{
	std::vector<std::string> names;

	std::ifstream file{ std::string( path ) };
	std::string   line;
	while( std::getline( file, line ) )
	{
		std::istringstream lineStream( line );
		std::string        keyword;
		std::string        name;
		if( !( lineStream >> keyword >> name ) || keyword != "newmtl" )
			continue;

		if( std::find( names.begin(), names.end(), name ) != names.end() )
		{
			GAIA_WARN( "Material: {} defines {} twice, using the first one", path, name );
			continue;
		}

		names.push_back( name );
	}

	return names;
}

bool Material::ReadMtl( std::string_view path, std::string_view materialName, MaterialData& outData )
{
	outData = MaterialData();

	std::ifstream file{ std::string( path ) };
	if( !file.is_open() )
	{
		GAIA_ERROR( "Material: failed to open {}", path );
		return false;
	}

	// Note: Lines only count while inside the wanted newmtl block, a file without newmtl lines is one material
	bool        bInside    = materialName.empty();
	bool        bFound     = materialName.empty();
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
			std::string name;
			lineStream >> name;

			if( bInside && !outData.name.empty() )
				break;

			bInside = materialName.empty() || name == materialName;
			if( bInside )
			{
				bFound       = true;
				outData.name = name;
			}
			continue;
		}

		if( !bInside )
			continue;

		if( keyword == "Kd" )
		{
			if( !ParseVec3( lineStream, outData.diffuseColor ) )
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
				outData.diffuseTexture = word;
			}
		}
		else if( keyword == "shader" )
		{
			lineStream >> outData.shader;
		}
	}

	if( !bFound )
	{
		GAIA_ERROR( "Material: {} has no material named {}", path, materialName );
		return false;
	}

	return true;
}

shared<Material> Material::FindOrError( std::string_view materialName, std::string_view requester )
{
	ResourceManager& manager = App::GetSession().GetResourceManager();

	if( shared<Material> material = manager.FindResource<Material>( materialName, true ) )
		return material;

	GAIA_WARN( "{}: material {} is not available, using {}", requester, materialName, kErrorName );

	shared<Material> errorMaterial = manager.FindResource<Material>( kErrorName, true );
	if( errorMaterial == nullptr )
	{
		GAIA_ERROR( "{}: the {} material is not available either, so nothing draws in its place", requester, kErrorName );
	}

	return errorMaterial;
}

bool Material::LoadInternal()
{
	if( !ReadMtl( GetPath(), GetEntry(), m_Data ) )
		return false;

	ResourceManager& manager = App::GetSession().GetResourceManager();

	m_Shader = manager.FindResource<Shader>( m_Data.shader, true );
	if( m_Shader == nullptr )
	{
		GAIA_ERROR( "Material {}: shader {} is not available", GetName(), m_Data.shader );
		return false;
	}

	if( !m_Data.diffuseTexture.empty() )
	{
		m_DiffuseTexture = Texture::FindOrError( m_Data.diffuseTexture, "Material " + std::string( GetName() ) );
	}

	return true;
}

bool Material::UnloadInternal()
{
	m_Shader.reset();
	m_DiffuseTexture.reset();
	return true;
}

void Material::Bind( const DrawContext& context ) const
{
	if( m_Shader == nullptr )
		return;

	m_Shader->Use();
	m_Shader->SetMatrix44( "u_Model", context.model );
	m_Shader->SetMatrix44( "u_View", context.view );
	m_Shader->SetMatrix44( "u_Projection", context.projection );

	m_Shader->SetVec3( "u_Color", m_Data.diffuseColor );
	m_Shader->SetInt( "u_UseTexture", m_DiffuseTexture != nullptr ? 1 : 0 );
	if( m_DiffuseTexture != nullptr )
	{
		m_DiffuseTexture->Bind( kDiffuseTextureUnit );
		m_Shader->SetInt( "u_Texture", kDiffuseTextureUnit );
	}
}
