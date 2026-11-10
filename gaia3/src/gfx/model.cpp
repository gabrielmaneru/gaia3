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
constexpr u32 kTangentAttribute  = 3;

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

// Note: Faces without a material use the default library, named ones are looked up in the model's own libraries
const MaterialEntry* FindSlotMaterial( const std::vector<shared<Material>>& libraries, const Material* defaultLibrary, const std::string& slot, std::string_view requester )
{
	if( slot.empty() )
	{
		const MaterialEntry* defaultMaterial = defaultLibrary != nullptr ? defaultLibrary->FindEntry( "" ) : nullptr;
		if( defaultMaterial == nullptr )
		{
			GAIA_ERROR( "{}: the {} material is not available, so faces without a material won't draw", requester, Material::kDefaultName );
		}
		return defaultMaterial;
	}

	for( const shared<Material>& library : libraries )
	{
		if( const MaterialEntry* material = library->FindEntry( slot ) )
			return material;
	}

	GAIA_WARN( "{}: material {} is not in its material libraries, using {}", requester, slot, Material::kErrorName );
	return Material::FindErrorEntry( requester );
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

	// Note: OBJ files have no tangents, formats like glTF can bring their own
	if( data.tangents.size() != data.positions.size() )
	{
		data.ComputeTangents();
	}

	glGenVertexArrays( 1, &m_VertexArray );
	glBindVertexArray( m_VertexArray );
	{
		m_PositionBuffer = CreateAttributeBuffer( kPositionAttribute, data.positions );
		m_NormalBuffer   = CreateAttributeBuffer( kNormalAttribute, data.normals );
		m_UVBuffer       = CreateAttributeBuffer( kUVAttribute, data.uvs );
		m_TangentBuffer  = CreateAttributeBuffer( kTangentAttribute, data.tangents );

		glGenBuffers( 1, &m_IndexBuffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof( u32 ), data.indices.data(), GL_STATIC_DRAW );
	}
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	const std::string requester = "Model " + std::string( GetName() );
	ResourceManager&  manager   = App::GetSession().GetResourceManager();

	// Note: Libraries are found by file name like any material, wherever the .mtl file is
	for( const std::string& libraryFile : data.materialLibraries )
	{
		const std::string libraryName = std::filesystem::path( libraryFile ).stem().string();
		if( shared<Material> library = manager.FindResource<Material>( libraryName, true ) )
		{
			m_Libraries.push_back( library );
		}
		else
		{
			GAIA_WARN( "{}: material library {} is not available", requester, libraryFile );
		}
	}

	shared<Material> defaultLibrary = manager.FindResource<Material>( Material::kDefaultName, true );

	// Note: Each slot is looked up once, many meshes usually share a material
	std::unordered_map<std::string, const MaterialEntry*> slotMaterials;
	for( const MeshData& mesh : data.meshes )
	{
		for( const SubMeshData& subMeshData : mesh.subMeshes )
		{
			const auto [slotIt, bNewSlot] = slotMaterials.try_emplace( subMeshData.materialSlot );
			if( bNewSlot )
			{
				slotIt->second = FindSlotMaterial( m_Libraries, defaultLibrary.get(), subMeshData.materialSlot, requester );
			}

			SubMesh subMesh;
			subMesh.indexOffset  = subMeshData.indexOffset;
			subMesh.indexCount   = subMeshData.indexCount;
			subMesh.materialSlot = subMeshData.materialSlot;
			subMesh.material     = slotIt->second;
			m_SubMeshes.push_back( subMesh );
		}
	}

	// Note: The default library only counts as the model's own when some faces have no material
	if( defaultLibrary != nullptr && slotMaterials.contains( "" ) )
	{
		m_Libraries.push_back( defaultLibrary );
	}

	GAIA_INFO( "Model {}: {} vertices, {} triangles, {} meshes, {} sub-meshes", GetName(), data.positions.size(), data.indices.size() / 3, data.meshes.size(), m_SubMeshes.size() );

	return true;
}

bool Model::UnloadInternal()
{
	const u32 buffers[] = { m_PositionBuffer, m_NormalBuffer, m_UVBuffer, m_TangentBuffer, m_IndexBuffer };
	glDeleteBuffers( 5, buffers );
	glDeleteVertexArrays( 1, &m_VertexArray );

	m_VertexArray    = 0;
	m_PositionBuffer = 0;
	m_NormalBuffer   = 0;
	m_UVBuffer       = 0;
	m_TangentBuffer  = 0;
	m_IndexBuffer    = 0;

	m_SubMeshes.clear();
	m_Libraries.clear();

	return true;
}

bool Model::UsesMaterialLibrary( std::string_view libraryName ) const
{
	// Note: Compares the resources, so "models/sponza" and "sponza" are the same library when they are the same file
	const shared<Material> library = App::GetSession().GetResourceManager().FindResource<Material>( libraryName );
	return library != nullptr && std::find( m_Libraries.begin(), m_Libraries.end(), library ) != m_Libraries.end();
}

std::vector<const MaterialEntry*> Model::FindMaterials( const Material& library ) const
{
	const std::string requester = "Model " + std::string( GetName() );

	std::vector<const MaterialEntry*>                     materials;
	std::unordered_map<std::string, const MaterialEntry*> slotMaterials;
	for( const SubMesh& subMesh : m_SubMeshes )
	{
		const auto [slotIt, bNewSlot] = slotMaterials.try_emplace( subMesh.materialSlot );
		if( bNewSlot )
		{
			slotIt->second = library.FindEntry( subMesh.materialSlot );
			if( slotIt->second == nullptr )
			{
				GAIA_WARN( "{}: material {} is not in {}, using {}", requester, subMesh.materialSlot, library.GetName(), Material::kErrorName );
				slotIt->second = Material::FindErrorEntry( requester );
			}
		}

		materials.push_back( slotIt->second );
	}

	return materials;
}

void Model::Draw( const DrawContext& context, const std::vector<const MaterialEntry*>& overrideMaterials ) const
{
	const bool bOverride = overrideMaterials.size() == m_SubMeshes.size();

	glBindVertexArray( m_VertexArray );

	for( u64 i = 0; i < m_SubMeshes.size(); ++i )
	{
		const SubMesh&       subMesh  = m_SubMeshes[i];
		const MaterialEntry* material = bOverride ? overrideMaterials[i] : subMesh.material;
		if( material == nullptr )
			continue;

		material->Bind( context );

		const u64 byteOffset = static_cast<u64>( subMesh.indexOffset ) * sizeof( u32 );
		glDrawElements( GL_TRIANGLES, subMesh.indexCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>( byteOffset ) );
	}

	glBindVertexArray( 0 );
}
