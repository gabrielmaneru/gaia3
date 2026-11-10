#pragma once

#include <core/types.h>
#include <math/vector.h>

#include <string>
#include <vector>

// Note: A range of the model's indices drawn with one material
struct SubMeshData
{
	u32         indexOffset = 0;
	u32         indexCount  = 0;
	std::string materialSlot;
};

// Note: One object or group of the file, drawn as one sub-mesh per material
struct MeshData
{
	std::string              name;
	std::vector<SubMeshData> subMeshes;
};

// Note: What every importer fills in whatever the file format, bones will be added here for skinned models
struct ModelData
{
	// Note: One entry per vertex in every stream, shared by every mesh of the model
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs;

	// Note: The direction the texture's U runs along the surface, xyz, and in w the side the V direction is on, +1 or
	// -1, so the bitangent is cross( normal, tangent ) * w. Filled by ComputeTangents when the importer has none
	std::vector<Vec4> tangents;

	std::vector<u32>      indices;
	std::vector<MeshData> meshes;

	// Note: Material files the model names, the materials themselves are found by name in every scanned .mtl
	std::vector<std::string> materialLibraries;

	// Note: Works out tangents from positions, normals and UVs, for normal maps. Needs every other stream filled
	void ComputeTangents();
};
