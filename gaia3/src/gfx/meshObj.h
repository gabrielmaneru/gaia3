#pragma once

#include <core/types.h>
#include <math/vector.h>

#include <string_view>
#include <vector>

struct VertexBufferObj
{
	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;
	std::vector<Vec3> normals;
};

struct MeshObj
{
	MeshObj() = default;
	MeshObj( std::string_view path );

	VertexBufferObj  vertexBuffer;
	std::vector<u32> indices;
	bool             bIsValid = false;
};
