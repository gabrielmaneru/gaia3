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

	// Note: The map_d texture, a cut-out mask. Having one turns on alpha testing, see MaterialEntry
	std::string opacityTexture;

	// Note: A tangent-space normal map, from a map_Bump, bump or norm line
	std::string normalTexture;

	std::string shader = "default";

	// Note: From a "two_sided" line, a gaia3 extension. Both sides of each face are drawn, for single-sheet geometry
	// like leaves, and the back side is lit as its own surface
	bool bTwoSided = false;
};

// Note: One newmtl of a material library, ready to draw with
struct MaterialEntry
{
	MaterialData    data;
	shared<Shader>  shader;
	shared<Texture> diffuseTexture;

	// Note: The map_d texture. Its alpha is the opacity when it has an alpha channel, so map_d can name the colour
	// texture itself, otherwise its red channel is, as for grey masks. Null when the mask can't be found, then the
	// diffuse texture's alpha is used
	shared<Texture> opacityTexture;

	// Note: Null without a normal map, or when it can't be found, then the surface uses the model's normals
	shared<Texture> normalTexture;

	// Note: Pixels whose opacity is under half aren't drawn, only for materials with a map_d line
	bool IsAlphaTested() const { return !data.opacityTexture.empty(); }

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

	// Note: Reads every newmtl of a .mtl file in order: its Kd colour, its textures, and the "shader" and "two_sided"
	// lines, gaia3 extensions other tools skip like any line they don't know. A file without newmtl lines is one
	// unnamed material
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
