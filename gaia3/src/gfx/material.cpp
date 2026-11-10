#include "gaia3_pch.h"
#include "material.h"

#include <core/app.h>
#include <gfx/shader.h>
#include <gfx/texture.h>

#include <glad/glad.h>

#include <algorithm>
#include <fstream>
#include <sstream>

RTTI_DEFINE_RESOURCE( Material,
                      RTTI_RESOURCE_FOLDER( "materials" ),
                      RTTI_RESOURCE_EXTENSIONS( ".mtl" ) )

namespace
{
constexpr u32 kDiffuseTextureUnit = 0;
constexpr u32 kOpacityTextureUnit = 1;
constexpr u32 kNormalTextureUnit  = 2;

bool ParseVec3( std::istringstream& stream, Vec3& outValue )
{
	return static_cast<bool>( stream >> outValue.x >> outValue.y >> outValue.z );
}

// Note: Options like "-s 1 1 1" come before the file name, so the file is the last word
std::string ReadTextureName( std::istringstream& stream )
{
	std::string word;
	std::string fileName;
	while( stream >> word )
	{
		fileName = word;
	}
	return fileName;
}
} // namespace

void MaterialEntry::Bind( const DrawContext& context ) const
{
	if( shader == nullptr )
		return;

	// Note: Every material sets culling itself, so a two-sided one never leaves it off for the next
	if( data.bTwoSided )
	{
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glEnable( GL_CULL_FACE );
	}

	shader->Use();
	shader->SetMatrix44( "u_Model", context.model );
	shader->SetMatrix44( "u_View", context.view );
	shader->SetMatrix44( "u_Projection", context.projection );
	shader->SetInt( "u_DrawMode", static_cast<int>( context.drawMode ) );

	const u64 lightCount = std::min<u64>( context.lights.size(), kMaxLights );
	shader->SetInt( "u_LightCount", static_cast<int>( lightCount ) );
	for( u64 i = 0; i < lightCount; ++i )
	{
		const LightParameters& light = context.lights[i];
		const std::string      index = "[" + std::to_string( i ) + "]";

		shader->SetInt( ( "u_LightType" + index ).c_str(), static_cast<int>( light.type ) );
		shader->SetVec3( ( "u_LightColor" + index ).c_str(), light.color );
		shader->SetVec3( ( "u_LightPosition" + index ).c_str(), light.position );
		shader->SetVec3( ( "u_LightDirection" + index ).c_str(), light.direction );
		shader->SetFloat( ( "u_LightRange" + index ).c_str(), light.range );
	}

	shader->SetVec3( "u_Color", data.diffuseColor );
	shader->SetInt( "u_UseTexture", diffuseTexture != nullptr ? 1 : 0 );
	if( diffuseTexture != nullptr )
	{
		diffuseTexture->Bind( kDiffuseTextureUnit );
		shader->SetInt( "u_Texture", kDiffuseTextureUnit );
	}

	shader->SetInt( "u_AlphaTest", IsAlphaTested() && context.bAlphaTest ? 1 : 0 );
	shader->SetInt( "u_UseOpacityTexture", opacityTexture != nullptr ? 1 : 0 );
	if( opacityTexture != nullptr )
	{
		opacityTexture->Bind( kOpacityTextureUnit );
		shader->SetInt( "u_OpacityTexture", kOpacityTextureUnit );
		shader->SetInt( "u_OpacityFromAlpha", opacityTexture->HasAlphaChannel() ? 1 : 0 );
	}

	shader->SetInt( "u_UseNormalTexture", normalTexture != nullptr ? 1 : 0 );
	if( normalTexture != nullptr )
	{
		normalTexture->Bind( kNormalTextureUnit );
		shader->SetInt( "u_NormalTexture", kNormalTextureUnit );
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
			currentMaterial().diffuseTexture = ReadTextureName( lineStream );
		}
		else if( keyword == "map_d" )
		{
			currentMaterial().opacityTexture = ReadTextureName( lineStream );
		}
		else if( keyword == "map_Bump" || keyword == "map_bump" || keyword == "bump" || keyword == "norm" )
		{
			currentMaterial().normalTexture = ReadTextureName( lineStream );
		}
		else if( keyword == "shader" )
		{
			lineStream >> currentMaterial().shader;
		}
		else if( keyword == "two_sided" )
		{
			currentMaterial().bTwoSided = true;
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

		// Note: A missing mask falls back to the diffuse texture's alpha rather than the error texture, which as a
		// mask would only punch a checker pattern of holes
		if( !data.opacityTexture.empty() )
		{
			entry.opacityTexture = manager.FindResource<Texture>( data.opacityTexture, true );
			if( entry.opacityTexture == nullptr )
			{
				GAIA_WARN( "Material {}: opacity texture {} of {} is not available, using the alpha of its colour texture", GetName(), data.opacityTexture, data.name );
			}
		}

		// Note: A missing normal map leaves the surface flat rather than using the error texture, which as a normal map
		// would only bend the lighting in a checker pattern
		if( !data.normalTexture.empty() )
		{
			entry.normalTexture = manager.FindResource<Texture>( data.normalTexture, true );
			if( entry.normalTexture == nullptr )
			{
				GAIA_WARN( "Material {}: normal map {} of {} is not available, using the model's normals", GetName(), data.normalTexture, data.name );
			}
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
