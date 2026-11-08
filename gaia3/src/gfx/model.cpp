#include "gaia3_pch.h"
#include "model.h"

#include <core/app.h>
#include <gfx/material.h>
#include <gfx/objImporter.h>

#include <glad/glad.h>

#include <filesystem>
#include <unordered_map>

RTTI_DEFINE_RESOURCE( Model,
                      RTTI_RESOURCE_FOLDER( "models" ),
                      RTTI_RESOURCE_EXTENSIONS( ".obj" ) )

namespace
{
constexpr u32 kPositionAttribute = 0;
constexpr u32 kNormalAttribute   = 1;
constexpr u32 kUVAttribute       = 2;

template <typename T>
u32 CreateAttributeBuffer( u32 attribute, const std::vector<T>& data )
{
	static_assert( sizeof( T ) % sizeof( float ) == 0, "Attribute types must only contain floats" );

	u32 buffer = 0;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, data.size() * sizeof( T ), data.data(), GL_STATIC_DRAW );

	glVertexAttribPointer( attribute, sizeof( T ) / sizeof( float ), GL_FLOAT, GL_FALSE, sizeof( T ), nullptr );
	glEnableVertexAttribArray( attribute );

	return buffer;
}

// Note: Picks the importer from the file extension, glTF and FBX importers will be added here
bool ImportModel( std::string_view path, ModelData& outData )
{
	const std::string extension = std::filesystem::path( path ).extension().string();
	if( extension == ".obj" || extension == ".OBJ" )
		return ObjImporter::Import( path, outData );

	GAIA_ERROR( "Model: no importer for {}", path );
	return false;
}

// Note: Faces without a material use the default one, and a named material that can't be found uses the error one
shared<Material> FindSlotMaterial( std::string_view modelName, const std::string& slot )
{
	const std::string requester = "Model " + std::string( modelName );
	if( !slot.empty() )
		return Material::FindOrError( slot, requester );

	shared<Material> defaultMaterial = App::GetSession().GetResourceManager().FindResource<Material>( Material::kDefaultName, true );
	if( defaultMaterial == nullptr )
	{
		GAIA_ERROR( "{}: the {} material is not available, so faces without a material won't draw", requester, Material::kDefaultName );
	}

	return defaultMaterial;
}

void WarnAboutMissingLibraries( std::string_view modelName, std::string_view modelPath, const std::vector<std::string>& libraries )
{
	const std::filesystem::path modelFolder = std::filesystem::path( modelPath ).parent_path();
	for( const std::string& library : libraries )
	{
		std::error_code ec;
		if( !std::filesystem::exists( modelFolder / library, ec ) )
		{
			GAIA_WARN( "Model {}: material library {} is not next to the model, its materials are looked up by name", modelName, library );
		}
	}
}
} // namespace

Model::Model()
    : Resource( "", ResourceState::New )
{
}

Model::Model( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

Model::~Model()
{
}

bool Model::LoadInternal()
{
	ModelData data;
	if( !ImportModel( GetPath(), data ) )
	{
		return false;
	}

	glGenVertexArrays( 1, &m_VertexArray );
	glBindVertexArray( m_VertexArray );
	{
		m_PositionBuffer = CreateAttributeBuffer( kPositionAttribute, data.positions );
		m_NormalBuffer   = CreateAttributeBuffer( kNormalAttribute, data.normals );
		m_UVBuffer       = CreateAttributeBuffer( kUVAttribute, data.uvs );

		glGenBuffers( 1, &m_IndexBuffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof( u32 ), data.indices.data(), GL_STATIC_DRAW );
	}
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	WarnAboutMissingLibraries( GetName(), GetPath(), data.materialLibraries );

	// Note: Each slot is looked up once, many meshes usually share a material
	std::unordered_map<std::string, shared<Material>> slotMaterials;
	for( const MeshData& mesh : data.meshes )
	{
		for( const SubMeshData& subMeshData : mesh.subMeshes )
		{
			const auto [slotIt, bNewSlot] = slotMaterials.try_emplace( subMeshData.materialSlot );
			if( bNewSlot )
			{
				slotIt->second = FindSlotMaterial( GetName(), subMeshData.materialSlot );
			}

			SubMesh subMesh;
			subMesh.indexOffset = subMeshData.indexOffset;
			subMesh.indexCount  = subMeshData.indexCount;
			subMesh.material    = slotIt->second;
			m_SubMeshes.push_back( subMesh );
		}
	}

	GAIA_INFO( "Model {}: {} vertices, {} triangles, {} meshes, {} sub-meshes", GetName(), data.positions.size(), data.indices.size() / 3, data.meshes.size(), m_SubMeshes.size() );

	return true;
}

bool Model::UnloadInternal()
{
	const u32 buffers[] = { m_PositionBuffer, m_NormalBuffer, m_UVBuffer, m_IndexBuffer };
	glDeleteBuffers( 4, buffers );
	glDeleteVertexArrays( 1, &m_VertexArray );

	m_VertexArray    = 0;
	m_PositionBuffer = 0;
	m_NormalBuffer   = 0;
	m_UVBuffer       = 0;
	m_IndexBuffer    = 0;

	m_SubMeshes.clear();

	return true;
}

void Model::Draw( const DrawContext& context, const Material* overrideMaterial ) const
{
	glBindVertexArray( m_VertexArray );

	for( const SubMesh& subMesh : m_SubMeshes )
	{
		const Material* material = overrideMaterial != nullptr ? overrideMaterial : subMesh.material.get();
		if( material == nullptr )
			continue;

		material->Bind( context );

		const u64 byteOffset = static_cast<u64>( subMesh.indexOffset ) * sizeof( u32 );
		glDrawElements( GL_TRIANGLES, subMesh.indexCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>( byteOffset ) );
	}

	glBindVertexArray( 0 );
}
