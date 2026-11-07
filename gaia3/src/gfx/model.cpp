#include "gaia3_pch.h"
#include "model.h"

#include <gfx/meshObj.h>

#include <glad/glad.h>

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
	MeshObj rawMesh( GetPath() );
	if( !rawMesh.bIsValid )
	{
		return false;
	}

	glGenVertexArrays( 1, &m_VertexArray );
	glBindVertexArray( m_VertexArray );
	{
		m_PositionBuffer = CreateAttributeBuffer( kPositionAttribute, rawMesh.vertexBuffer.positions );
		m_NormalBuffer   = CreateAttributeBuffer( kNormalAttribute, rawMesh.vertexBuffer.normals );
		m_UVBuffer       = CreateAttributeBuffer( kUVAttribute, rawMesh.vertexBuffer.uvs );

		glGenBuffers( 1, &m_IndexBuffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, rawMesh.indices.size() * sizeof( u32 ), rawMesh.indices.data(), GL_STATIC_DRAW );
		m_IndexCount = static_cast<u32>( rawMesh.indices.size() );

	}
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	GAIA_INFO( "Model {}: {} vertices, {} triangles", GetName(), rawMesh.vertexBuffer.positions.size(), rawMesh.indices.size() / 3 );

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
	m_IndexCount     = 0;

	return true;
}

void Model::Draw() const
{
	glBindVertexArray( m_VertexArray );
	glDrawElements( GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr );
	glBindVertexArray( 0 );
}
