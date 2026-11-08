#pragma once

#include <gfx/modelData.h>

#include <string_view>

// Note: Reads Wavefront .obj files into ModelData, Model is the resource that owns the result
class ObjImporter
{
public:
	// Note: Each object or group (o, g) becomes a mesh, split into one sub-mesh per material (usemtl). Repeated names
	// join the earlier mesh or sub-mesh, and mtllib names are kept in ModelData::materialLibraries
	static bool Import( std::string_view path, ModelData& outData );
};
