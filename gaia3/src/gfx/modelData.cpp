#include "modelData.h"

#include <math/mathHelpers.h>
#include <math/vectorTransforms.h>

#include <math.h>

namespace
{
constexpr float kDegenerateArea = 1e-12f;

// Note: Any direction along the surface, for vertices whose UVs don't give one
Vec3 AnyPerpendicular( const Vec3& normal )
{
	const Vec3 axis = fabsf( normal.x ) < 0.9f ? Vec3::V_Right : Vec3::V_Up;
	return Cross( axis, normal );
}

// Note: Indices that point past the vertices are skipped, so a half-edited model still gives a bounding volume
AABB GetIndexRangeBoundingVolume( const ModelData& data, u64 indexOffset, u64 indexCount )
{
	AABB      boundingVolume;
	const u64 end = Min<u64>( indexOffset + indexCount, data.indices.size() );
	for( u64 i = indexOffset; i < end; ++i )
	{
		const u32 vertex = data.indices[i];
		if( vertex < data.positions.size() )
		{
			boundingVolume.Add( data.positions[vertex] );
		}
	}
	return boundingVolume;
}
} // namespace

AABB ModelData::GetBoundingVolume() const
{
	return GetIndexRangeBoundingVolume( *this, 0, indices.size() );
}

AABB ModelData::GetMeshBoundingVolume( u64 meshIndex ) const
{
	AABB boundingVolume;
	if( meshIndex >= meshes.size() )
		return boundingVolume;

	for( const SubMeshData& subMesh : meshes[meshIndex].subMeshes )
	{
		boundingVolume.Add( GetIndexRangeBoundingVolume( *this, subMesh.indexOffset, subMesh.indexCount ) );
	}
	return boundingVolume;
}

AABB ModelData::GetSubMeshBoundingVolume( u64 meshIndex, u64 subMeshIndex ) const
{
	if( meshIndex >= meshes.size() || subMeshIndex >= meshes[meshIndex].subMeshes.size() )
		return AABB();

	const SubMeshData& subMesh = meshes[meshIndex].subMeshes[subMeshIndex];
	return GetIndexRangeBoundingVolume( *this, subMesh.indexOffset, subMesh.indexCount );
}

void ModelData::ComputeTangents()
{
	std::vector<Vec3> uDirections( positions.size(), Vec3::V_Zero );
	std::vector<Vec3> vDirections( positions.size(), Vec3::V_Zero );

	// Note: Each triangle adds the directions its UVs run along its surface to its three corners
	for( u64 i = 0; i + 2 < indices.size(); i += 3 )
	{
		const u32 a = indices[i];
		const u32 b = indices[i + 1];
		const u32 c = indices[i + 2];

		const Vec3 edge1 = positions[b] - positions[a];
		const Vec3 edge2 = positions[c] - positions[a];

		const float du1 = uvs[b].x - uvs[a].x;
		const float dv1 = uvs[b].y - uvs[a].y;
		const float du2 = uvs[c].x - uvs[a].x;
		const float dv2 = uvs[c].y - uvs[a].y;

		// Note: Triangles whose UVs are squashed to a line or a point give no direction
		const float uvArea = du1 * dv2 - du2 * dv1;
		if( fabsf( uvArea ) < kDegenerateArea )
			continue;

		const Scalar inverseArea( 1.0f / uvArea );
		const Vec3   uDirection = ( edge1 * Scalar( dv2 ) - edge2 * Scalar( dv1 ) ) * inverseArea;
		const Vec3   vDirection = ( edge2 * Scalar( du1 ) - edge1 * Scalar( du2 ) ) * inverseArea;

		for( const u32 corner : { a, b, c } )
		{
			uDirections[corner] += uDirection;
			vDirections[corner] += vDirection;
		}
	}

	tangents.assign( positions.size(), Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	for( u64 i = 0; i < positions.size(); ++i )
	{
		const Vec3& normal = normals[i];
		if( normal.LengthSq() < kDegenerateArea )
			continue;

		// Note: Keeps the part of the U direction that lies along the surface, at a right angle to the normal
		const Vec3 unitNormal = normal.Normalized();
		Vec3       tangent    = uDirections[i] - unitNormal * Scalar( Dot( unitNormal, uDirections[i] ) );
		if( tangent.LengthSq() < kDegenerateArea )
		{
			tangent = AnyPerpendicular( unitNormal );
		}
		tangent.Normalize();

		// Note: Mirrored UVs make V run the other way, the sign tells the shader to flip the bitangent
		const float handedness = Dot( Cross( unitNormal, tangent ), vDirections[i] ) < 0.0f ? -1.0f : 1.0f;
		tangents[i]            = Vec4( tangent, handedness );
	}
}
