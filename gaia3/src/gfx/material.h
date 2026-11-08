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

// Note: One material of a .mtl file in materials/, a file can hold several and each one is named by its newmtl line
class Material : public Resource
{
	RTTI_DECLARE_RESOURCE( Material )
public:
	static constexpr const char* kDefaultName = "default";
	static constexpr const char* kErrorName   = "error";

	explicit Material( std::string_view path, ResourceState state = ResourceState::NotLoaded );

	// Note: The names of every newmtl in the file, in order
	static std::vector<std::string> ListEntries( std::string_view path );

	// Note: Reads one material of a .mtl file, the first one when materialName is empty: its Kd colour, its map_Kd
	// texture and a "shader" line, a gaia3 extension other tools skip like any line they don't know
	static bool ReadMtl( std::string_view path, std::string_view materialName, MaterialData& outData );

	// Note: Returns the named material, or the error material with a warning so a missing one stands out
	static shared<Material> FindOrError( std::string_view materialName, std::string_view requester );

	bool LoadInternal() override;
	bool UnloadInternal() override;

	// Note: Selects the shader, then sets the material's values and the object's matrices
	void Bind( const DrawContext& context ) const;

private:
	MaterialData    m_Data;
	shared<Shader>  m_Shader;
	shared<Texture> m_DiffuseTexture;
};
