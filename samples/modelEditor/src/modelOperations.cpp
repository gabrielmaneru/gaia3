#include "modelOperations.h"

#include <math/mathHelpers.h>
#include <math/vectorTransforms.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
// Note: One sub-mesh's triangles as their own list, so meshes can be rearranged before being packed back together
struct SubMeshIndices
{
	std::string      materialSlot;
	std::vector<u32> indices;
};

struct MeshIndices
{
	std::string                 name;
	std::vector<SubMeshIndices> subMeshes;
};

std::vector<MeshIndices> Unpack( const ModelData& data )
{
	std::vector<MeshIndices> meshes;
	for( const MeshData& mesh : data.meshes )
	{
		MeshIndices meshIndices{ mesh.name, {} };
		for( const SubMeshData& subMesh : mesh.subMeshes )
		{
			const u64 begin = Min<u64>( subMesh.indexOffset, data.indices.size() );
			const u64 end   = Min<u64>( begin + subMesh.indexCount, data.indices.size() );
			meshIndices.subMeshes.push_back( SubMeshIndices{ subMesh.materialSlot, std::vector<u32>( data.indices.begin() + begin, data.indices.begin() + end ) } );
		}
		meshes.push_back( std::move( meshIndices ) );
	}
	return meshes;
}

// Note: Lays the meshes out one after another again. Sub-meshes of a mesh sharing a material join into one, like the
// importer makes them, and meshes or sub-meshes left without triangles are dropped
void Pack( ModelData& data, const std::vector<MeshIndices>& meshes )
{
	data.indices.clear();
	data.meshes.clear();

	for( const MeshIndices& mesh : meshes )
	{
		std::vector<SubMeshIndices> joined;
		for( const SubMeshIndices& subMesh : mesh.subMeshes )
		{
			auto sameSlot = std::find_if( joined.begin(), joined.end(), [&subMesh]( const SubMeshIndices& existing ) {
				return existing.materialSlot == subMesh.materialSlot;
			} );

			if( sameSlot != joined.end() )
			{
				sameSlot->indices.insert( sameSlot->indices.end(), subMesh.indices.begin(), subMesh.indices.end() );
			}
			else
			{
				joined.push_back( subMesh );
			}
		}

		MeshData meshData;
		meshData.name = mesh.name;
		for( const SubMeshIndices& subMesh : joined )
		{
			if( subMesh.indices.empty() )
				continue;

			meshData.subMeshes.push_back( SubMeshData{ static_cast<u32>( data.indices.size() ), static_cast<u32>( subMesh.indices.size() ), subMesh.materialSlot } );
			data.indices.insert( data.indices.end(), subMesh.indices.begin(), subMesh.indices.end() );
		}

		if( !meshData.subMeshes.empty() )
		{
			data.meshes.push_back( std::move( meshData ) );
		}
	}
}

// Note: The exact bits of a vector, so equal vectors give equal keys
std::array<u32, 3> GetBits( const Vec3& value )
{
	std::array<u32, 3> bits;
	std::memcpy( &bits[0], &value.x, sizeof( float ) );
	std::memcpy( &bits[1], &value.y, sizeof( float ) );
	std::memcpy( &bits[2], &value.z, sizeof( float ) );
	return bits;
}

i64 Quantize( float value, float step )
{
	return static_cast<i64>( std::llround( value / step ) );
}

Vec3 NormalizedOr( const Vec3& value, const Vec3& fallback )
{
	return value.LengthSq() > 1e-20f ? value.Normalized() : fallback;
}
} // namespace

void ModelOperations::Transform( ModelData& data, const Matrix44& transform )
{
	const Matrix33 linear( Vec3( transform.col[0] ), Vec3( transform.col[1] ), Vec3( transform.col[2] ) );
	const float    determinant = linear.Determinant();

	for( Vec3& position : data.positions )
	{
		position = Vec3( transform * Vec4( position, 1.0f ) );
	}

	// Note: A flattening transform has no inverse, the normals are left as they were
	if( fabsf( determinant ) > 1e-12f )
	{
		// Note: The inverse transpose, the inverse's rows used as columns
		const Matrix33 inverse = linear.Inverse();
		const Matrix33 normalMatrix( inverse.GetRow( 0 ), inverse.GetRow( 1 ), inverse.GetRow( 2 ) );
		for( Vec3& normal : data.normals )
		{
			normal = NormalizedOr( normalMatrix * normal, normal );
		}
	}

	if( determinant < 0.0f )
	{
		for( u64 i = 0; i + 2 < data.indices.size(); i += 3 )
		{
			std::swap( data.indices[i + 1], data.indices[i + 2] );
		}
	}

	data.tangents.clear();
}

void ModelOperations::FlipFaces( ModelData& data )
{
	for( u64 i = 0; i + 2 < data.indices.size(); i += 3 )
	{
		std::swap( data.indices[i + 1], data.indices[i + 2] );
	}

	for( Vec3& normal : data.normals )
	{
		normal = -normal;
	}

	data.tangents.clear();
}

void ModelOperations::ComputeNormals( ModelData& data, float smoothingAngle )
{
	const u64 triangleCount = data.indices.size() / 3;

	// Note: Each face counts by the angle it makes at a corner, so how a surface was split into triangles doesn't
	// change its normals, and a cube's smooth corners point exactly along its diagonals
	std::vector<Vec3>  unitFaceNormals( triangleCount );
	std::vector<float> cornerAngles( triangleCount * 3 );
	for( u64 t = 0; t < triangleCount; ++t )
	{
		const Vec3 corners[3] = { data.positions[data.indices[t * 3]], data.positions[data.indices[t * 3 + 1]], data.positions[data.indices[t * 3 + 2]] };
		unitFaceNormals[t]    = NormalizedOr( Cross( corners[1] - corners[0], corners[2] - corners[0] ), Vec3::V_Zero );

		for( u64 corner = 0; corner < 3; ++corner )
		{
			const Vec3 toNext     = NormalizedOr( corners[( corner + 1 ) % 3] - corners[corner], Vec3::V_Zero );
			const Vec3 toPrevious = NormalizedOr( corners[( corner + 2 ) % 3] - corners[corner], Vec3::V_Zero );
			cornerAngles[t * 3 + corner] = acosf( Clamp( Dot( toNext, toPrevious ), -1.0f, 1.0f ) );
		}
	}

	// Note: The triangle corners at each position, found by the position itself, since vertices are already split
	// wherever UVs or old normals differed
	std::map<std::array<u32, 3>, std::vector<u64>> cornersAtPosition;
	for( u64 cornerIndex = 0; cornerIndex < triangleCount * 3; ++cornerIndex )
	{
		cornersAtPosition[GetBits( data.positions[data.indices[cornerIndex]] )].push_back( cornerIndex );
	}

	// Note: A small allowance, so faces exactly at the limit, or flat faces of the same plane, still count
	const float minimumCosine = cosf( ToRadians( Clamp( smoothingAngle, 0.0f, 180.0f ) ) ) - 1e-4f;

	const bool        bHasUVs     = data.uvs.size() == data.positions.size();
	const bool        bHasNormals = data.normals.size() == data.positions.size();
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs;
	std::vector<u32>  indices( data.indices.size() );

	// Note: A vertex keeps one copy per different normal it ends up with, so hard edges split it
	std::map<std::pair<u32, std::array<u32, 3>>, u32> newVertices;
	for( u64 t = 0; t < triangleCount; ++t )
	{
		for( u64 corner = 0; corner < 3; ++corner )
		{
			const u32 vertex = data.indices[t * 3 + corner];

			Vec3 sum = Vec3::V_Zero;
			for( const u64 otherCorner : cornersAtPosition[GetBits( data.positions[vertex] )] )
			{
				const u64 other = otherCorner / 3;
				if( Dot( unitFaceNormals[t], unitFaceNormals[other] ) >= minimumCosine )
				{
					sum += unitFaceNormals[other] * Scalar( cornerAngles[otherCorner] );
				}
			}

			const Vec3 fallback = bHasNormals ? data.normals[vertex] : Vec3::V_Up;
			const Vec3 normal   = NormalizedOr( sum, fallback );

			const auto [vertexIt, bNew] = newVertices.try_emplace( std::make_pair( vertex, GetBits( normal ) ), static_cast<u32>( positions.size() ) );
			if( bNew )
			{
				positions.push_back( data.positions[vertex] );
				normals.push_back( normal );
				uvs.push_back( bHasUVs ? data.uvs[vertex] : Vec2() );
			}
			indices[t * 3 + corner] = vertexIt->second;
		}
	}

	data.positions = std::move( positions );
	data.normals   = std::move( normals );
	data.uvs       = std::move( uvs );
	data.indices   = std::move( indices );
	data.tangents.clear();
}

void ModelOperations::WeldVertices( ModelData& data, float tolerance )
{
	const float step        = Max( tolerance, 1e-7f );
	const bool  bHasNormals = data.normals.size() == data.positions.size();
	const bool  bHasUVs     = data.uvs.size() == data.positions.size();

	// Note: Vertices are compared on a grid tolerance wide, the first one in each cell is kept
	std::map<std::array<i64, 8>, u32> kept;
	std::vector<u32>                  remap( data.positions.size() );
	for( u64 i = 0; i < data.positions.size(); ++i )
	{
		const Vec3& position = data.positions[i];
		const Vec3  normal   = bHasNormals ? data.normals[i] : Vec3::V_Zero;
		const Vec2  uv       = bHasUVs ? data.uvs[i] : Vec2();

		const std::array<i64, 8> key = {
		    Quantize( position.x, step ), Quantize( position.y, step ), Quantize( position.z, step ),
		    Quantize( normal.x, step ), Quantize( normal.y, step ), Quantize( normal.z, step ),
		    Quantize( uv.x, step ), Quantize( uv.y, step ),
		};
		remap[i] = kept.try_emplace( key, static_cast<u32>( i ) ).first->second;
	}

	for( u32& index : data.indices )
	{
		if( index < remap.size() )
		{
			index = remap[index];
		}
	}

	data.tangents.clear();
	RemoveUnusedVertices( data );
}

void ModelOperations::RemoveUnusedVertices( ModelData& data )
{
	std::vector<u32> newIndex( data.positions.size(), ~0u );
	u32              usedCount = 0;
	for( const u32 index : data.indices )
	{
		if( index < newIndex.size() && newIndex[index] == ~0u )
		{
			newIndex[index] = usedCount++;
		}
	}

	// Note: Every stream is compacted the same way, the ones that don't hold one entry per vertex are left as they are
	auto compact = [&newIndex, usedCount]( auto& stream ) {
		if( stream.size() != newIndex.size() )
			return;

		std::remove_reference_t<decltype( stream )> compacted( usedCount );
		for( u64 i = 0; i < newIndex.size(); ++i )
		{
			if( newIndex[i] != ~0u )
			{
				compacted[newIndex[i]] = stream[i];
			}
		}
		stream = std::move( compacted );
	};

	compact( data.normals );
	compact( data.uvs );
	compact( data.tangents );
	compact( data.positions );

	for( u32& index : data.indices )
	{
		if( index < newIndex.size() )
		{
			index = newIndex[index];
		}
	}
}

bool ModelOperations::RenameMesh( ModelData& data, u64 meshIndex, std::string_view name )
{
	if( meshIndex >= data.meshes.size() || data.meshes[meshIndex].name == name )
		return false;

	data.meshes[meshIndex].name = std::string( name );
	return true;
}

bool ModelOperations::DeleteMesh( ModelData& data, u64 meshIndex )
{
	if( meshIndex >= data.meshes.size() )
		return false;

	std::vector<MeshIndices> meshes = Unpack( data );
	meshes.erase( meshes.begin() + static_cast<i64>( meshIndex ) );
	Pack( data, meshes );
	RemoveUnusedVertices( data );
	return true;
}

bool ModelOperations::MergeMeshes( ModelData& data, u64 targetMeshIndex, u64 sourceMeshIndex )
{
	if( targetMeshIndex >= data.meshes.size() || sourceMeshIndex >= data.meshes.size() || targetMeshIndex == sourceMeshIndex )
		return false;

	std::vector<MeshIndices> meshes = Unpack( data );
	MeshIndices&             target = meshes[targetMeshIndex];
	MeshIndices&             source = meshes[sourceMeshIndex];
	target.subMeshes.insert( target.subMeshes.end(), source.subMeshes.begin(), source.subMeshes.end() );
	meshes.erase( meshes.begin() + static_cast<i64>( sourceMeshIndex ) );
	Pack( data, meshes );
	return true;
}

bool ModelOperations::SplitSubMesh( ModelData& data, u64 meshIndex, u64 subMeshIndex )
{
	// Note: A mesh with a single sub-mesh is already on its own
	if( meshIndex >= data.meshes.size() || subMeshIndex >= data.meshes[meshIndex].subMeshes.size() || data.meshes[meshIndex].subMeshes.size() < 2 )
		return false;

	std::vector<MeshIndices> meshes = Unpack( data );
	MeshIndices&             mesh   = meshes[meshIndex];

	MeshIndices split;
	split.subMeshes.push_back( mesh.subMeshes[subMeshIndex] );
	split.name = mesh.name + "_" + ( split.subMeshes[0].materialSlot.empty() ? std::string( "default" ) : split.subMeshes[0].materialSlot );
	mesh.subMeshes.erase( mesh.subMeshes.begin() + static_cast<i64>( subMeshIndex ) );

	meshes.insert( meshes.begin() + static_cast<i64>( meshIndex ) + 1, std::move( split ) );
	Pack( data, meshes );
	return true;
}

bool ModelOperations::SetMaterialSlot( ModelData& data, u64 meshIndex, u64 subMeshIndex, std::string_view materialSlot )
{
	if( meshIndex >= data.meshes.size() || subMeshIndex >= data.meshes[meshIndex].subMeshes.size() || data.meshes[meshIndex].subMeshes[subMeshIndex].materialSlot == materialSlot )
		return false;

	std::vector<MeshIndices> meshes = Unpack( data );
	meshes[meshIndex].subMeshes[subMeshIndex].materialSlot = std::string( materialSlot );
	Pack( data, meshes );
	return true;
}
