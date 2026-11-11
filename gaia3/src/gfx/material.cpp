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
constexpr u32 kShadowMapUnit      = 3;
constexpr u32 kRoughnessUnit      = 4;
constexpr u32 kMetallicUnit       = 5;

// Note: Optional textures that leave the material without them rather than using the error texture, which would only
// bend or tint the result in a checker pattern
shared<Texture> FindOptionalTexture( ResourceManager& manager, const std::string& textureName, std::string_view what, std::string_view materialName, std::string_view libraryName )
{
	if( textureName.empty() )
		return nullptr;

	shared<Texture> texture = manager.FindResource<Texture>( textureName, true );
	if( texture == nullptr )
	{
		GAIA_WARN( "Material {}: {} {} of {} is not available, drawing without it", libraryName, what, textureName, materialName );
	}
	return texture;
}

// Note: Binds an optional texture and tells the shader whether there is one
void BindOptionalTexture( const Shader& shader, const shared<Texture>& texture, u32 unit, const char* useName, const char* samplerName )
{
	shader.SetInt( useName, texture != nullptr ? 1 : 0 );
	if( texture != nullptr )
	{
		texture->Bind( unit );
		shader.SetInt( samplerName, static_cast<int>( unit ) );
	}
}

// Note: Every material sets culling itself, so a two-sided one never leaves it off for the next
void SetCulling( bool bTwoSided )
{
	if( bTwoSided )
	{
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glEnable( GL_CULL_FACE );
	}
}

// Note: The values that decide which pixels a cut-out material drops, shared by the colour and shadow passes
void BindCutOut( const MaterialEntry& material, const Shader& shader, bool bAlphaTest )
{
	shader.SetInt( "u_UseTexture", material.diffuseTexture != nullptr ? 1 : 0 );
	if( material.diffuseTexture != nullptr )
	{
		material.diffuseTexture->Bind( kDiffuseTextureUnit );
		shader.SetInt( "u_Texture", kDiffuseTextureUnit );
	}

	shader.SetInt( "u_AlphaTest", bAlphaTest ? 1 : 0 );
	shader.SetInt( "u_UseOpacityTexture", material.opacityTexture != nullptr ? 1 : 0 );
	if( material.opacityTexture != nullptr )
	{
		material.opacityTexture->Bind( kOpacityTextureUnit );
		shader.SetInt( "u_OpacityTexture", kOpacityTextureUnit );
		shader.SetInt( "u_OpacityFromAlpha", material.opacityTexture->HasAlphaChannel() ? 1 : 0 );
	}
}

bool ParseVec3( std::istringstream& stream, Vec3& outValue )
{
	return static_cast<bool>( stream >> outValue.x >> outValue.y >> outValue.z );
}

bool ParseFloat( std::istringstream& stream, float& outValue )
{
	return static_cast<bool>( stream >> outValue );
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

float MaterialData::GetRoughness() const
{
	if( roughness >= 0.0f )
		return roughness;

	if( !roughnessTexture.empty() )
		return 1.0f;

	// Note: The usual match between a Blinn-Phong exponent and GGX roughness, a medium 0.5 without one
	if( shininess > 0.0f )
		return sqrtf( 2.0f / ( shininess + 2.0f ) );

	return 0.5f;
}

float MaterialData::GetMetallic() const
{
	if( metallic >= 0.0f )
		return metallic;

	return metallicTexture.empty() ? 0.0f : 1.0f;
}

void MaterialEntry::Bind( const DrawContext& context ) const
{
	if( shader == nullptr )
		return;

	SetCulling( data.bTwoSided );

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

	shader->SetVec3( "u_CameraPosition", context.cameraPosition );
	shader->SetVec3( "u_Color", data.diffuseColor );
	BindCutOut( *this, *shader, IsAlphaTested() && context.bAlphaTest );

	BindOptionalTexture( *shader, normalTexture, kNormalTextureUnit, "u_UseNormalTexture", "u_NormalTexture" );

	shader->SetFloat( "u_Roughness", data.GetRoughness() );
	shader->SetFloat( "u_Metallic", data.GetMetallic() );
	BindOptionalTexture( *shader, roughnessTexture, kRoughnessUnit, "u_UseRoughnessTexture", "u_RoughnessTexture" );
	BindOptionalTexture( *shader, metallicTexture, kMetallicUnit, "u_UseMetallicTexture", "u_MetallicTexture" );

	// Note: The shadow sampler always gets its own unit, a sampler2DShadow sharing one with a sampler2D fails to draw
	shader->SetInt( "u_ShadowMap", kShadowMapUnit );
	shader->SetInt( "u_ShadowLight", context.shadowLight );
	if( context.shadowLight >= 0 )
	{
		glActiveTexture( GL_TEXTURE0 + kShadowMapUnit );
		glBindTexture( GL_TEXTURE_2D, context.shadowMap );
		shader->SetMatrix44( "u_ShadowMatrix", context.shadowMatrix );
		shader->SetFloat( "u_ShadowDepthBias", context.shadowDepthBias );
		shader->SetFloat( "u_ShadowNormalOffset", context.shadowNormalOffset );
	}
}

void MaterialEntry::BindShadow( const DrawContext& context, const Shader& shadowShader ) const
{
	SetCulling( data.bTwoSided );

	shadowShader.Use();
	shadowShader.SetMatrix44( "u_Model", context.model );
	shadowShader.SetMatrix44( "u_ShadowMatrix", context.shadowMatrix );
	BindCutOut( *this, shadowShader, IsAlphaTested() && context.bAlphaTest );
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
		else if( keyword == "Ns" || keyword == "Pr" || keyword == "Pm" )
		{
			MaterialData& material = currentMaterial();
			float&        value    = keyword == "Ns" ? material.shininess : ( keyword == "Pr" ? material.roughness : material.metallic );
			if( !ParseFloat( lineStream, value ) )
			{
				GAIA_ERROR( "Material: invalid {} at {}:{}", keyword, path, lineNumber );
				return false;
			}
		}
		else if( keyword == "map_Pr" )
		{
			currentMaterial().roughnessTexture = ReadTextureName( lineStream );
		}
		else if( keyword == "map_Pm" )
		{
			currentMaterial().metallicTexture = ReadTextureName( lineStream );
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

		entry.normalTexture    = FindOptionalTexture( manager, data.normalTexture, "normal map", data.name, GetName() );
		entry.roughnessTexture = FindOptionalTexture( manager, data.roughnessTexture, "roughness texture", data.name, GetName() );
		entry.metallicTexture  = FindOptionalTexture( manager, data.metallicTexture, "metallic texture", data.name, GetName() );

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
