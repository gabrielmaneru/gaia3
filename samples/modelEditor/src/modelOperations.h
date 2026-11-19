#pragma once

#include <core/types.h>
#include <gfx/modelData.h>
#include <math/matrix.h>

#include <string_view>

// Note: The Model Editor's edits on ModelData, made while preparing a model rather than while a game runs. They change
// only the data, building a Model from it is up to the caller. Edits that move vertices or change their normals clear the tangents, Model::Build works them out
// again. Mesh edits return false, changing nothing, for an index out of range
namespace ModelOperations
{
// Note: Moves, turns and scales every vertex. Normals follow with the inverse transpose, so they stay at right angles
// to the surface even under uneven scale. A mirroring transform also reverses the winding, so faces keep facing out
void Transform( ModelData& data, const Matrix44& transform );

// Note: Turns every face around, reversed winding and normals pointing the other way
void FlipFaces( ModelData& data );

// Note: Works out the normals from the faces, each face counting by the angle it makes at the corner. Faces meeting at
// up to smoothingAngle degrees share smooth normals and sharper edges stay hard, 0 gives flat faces and 180 smooths
// everything. Vertices are split where an edge turns hard
void ComputeNormals( ModelData& data, float smoothingAngle );

// Note: Merges vertices whose position, normal and UV all match to within tolerance, then drops unused ones
void WeldVertices( ModelData& data, float tolerance );

// Note: Drops the vertices no triangle uses
void RemoveUnusedVertices( ModelData& data );

bool RenameMesh( ModelData& data, u64 meshIndex, std::string_view name );

// Note: Removes the mesh's triangles, and the vertices only they used
bool DeleteMesh( ModelData& data, u64 meshIndex );

// Note: Moves the source mesh's triangles into the target mesh. Sub-meshes with the same material join into one
bool MergeMeshes( ModelData& data, u64 targetMeshIndex, u64 sourceMeshIndex );

// Note: Moves one sub-mesh into a new mesh of its own, placed after its old mesh and named after both
bool SplitSubMesh( ModelData& data, u64 meshIndex, u64 subMeshIndex );

// Note: Draws the sub-mesh with another material of the model's libraries, empty for the default one. It joins a
// sub-mesh of the same mesh that already uses that material
bool SetMaterialSlot( ModelData& data, u64 meshIndex, u64 subMeshIndex, std::string_view materialSlot );
} // namespace ModelOperations
