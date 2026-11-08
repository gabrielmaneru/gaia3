#pragma once

#include <gfx/rendererTypes.h>
#include <math/vector.h>
#include <system/resource.h>

#include <string>
#include <string_view>
#include <vector>

class Shader;
class Texture;

struct MaterialData
{
	std::string name;
	Vec3        diffuseColor = Vec3( 1.0f, 1.0f, 1.0f );
	std::string diffuseTexture;
	std::string shader = "default";
};

// Note: One newmtl of a material library, ready to draw with
struct MaterialEntry
{
	MaterialData    data;
	shared<Shader>  shader;
	shared<Texture> diffuseTexture;

	// Note: Selects the shader, then sets the material's values and the object's matrices
	void Bind( const DrawContext& context ) const;
};

// Note: A .mtl file in materials/, named after the file. Its materials are only seen by the models that use the
// library, so names like "floor" in different libraries never clash
class Material : public Resource
{
	RTTI_DECLARE_RESOURCE( Material )
public:
	static constexpr const char* kDefaultName = "default";
	static constexpr const char* kErrorName   = "error";

	explicit Material( std::string_view path, ResourceState state = ResourceState::NotLoaded );

	// Note: Reads every newmtl of a .mtl file in order: its Kd colour, its map_Kd texture and a "shader" line, a
	// gaia3 extension other tools skip like any line they don't know. A file without newmtl lines is one unnamed
	// material
	static bool ReadMtl( std::string_view path, std::vector<MaterialData>& outMaterials );

	// Note: Returns the named library, or the error library with a warning so a missing one stands out
	static shared<Material> FindOrError( std::string_view libraryName, std::string_view requester );

	// Note: The error library's material, for anything whose material can't be found. The ResourceManager keeps the
	// library loaded until it shuts down, so callers can keep the pointer without holding the library themselves
	static const MaterialEntry* FindErrorEntry( std::string_view requester );

	bool LoadInternal() override;
	bool UnloadInternal() override;

	// Note: The material with that name, or the first one when the name is empty. Null when there is none, entries
	// stay valid while the library is loaded
	const MaterialEntry*              FindEntry( std::string_view materialName ) const;
	const std::vector<MaterialEntry>& GetEntries() const { return m_Entries; }

private:
	std::vector<MaterialEntry> m_Entries;
};
