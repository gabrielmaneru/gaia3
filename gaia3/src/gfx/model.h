#pragma once

#include <gfx/rendererTypes.h>
#include <system/resource.h>

#include <string>
#include <vector>

class Material;
struct MaterialEntry;

class Model : public Resource
{
	RTTI_DECLARE_RESOURCE( Model )
public:
	Model();
	explicit Model( std::string_view path, ResourceState state = ResourceState::NotLoaded );
	~Model();

	bool LoadInternal() override;
	bool UnloadInternal() override;

	// Note: True for the libraries the model already draws with: its mtllib libraries, and default when some faces
	// have no material. Overriding with one of them would change nothing
	bool UsesMaterialLibrary( std::string_view libraryName ) const;

	// Note: The material of each sub-mesh, in order, found by its usemtl name in library instead of the model's own
	// libraries. Sub-meshes without a name use the library's first material, and missing names use the error one
	std::vector<const MaterialEntry*> FindMaterials( const Material& library ) const;

	// Note: Draws each sub-mesh with its own material, or with overrideMaterials when it holds one per sub-mesh
	void Draw( const DrawContext& context, const std::vector<const MaterialEntry*>& overrideMaterials ) const;

private:
	struct SubMesh
	{
		u32                  indexOffset = 0;
		u32                  indexCount  = 0;
		std::string          materialSlot;
		const MaterialEntry* material = nullptr;
	};

	u32 m_VertexArray    = 0;
	u32 m_PositionBuffer = 0;
	u32 m_NormalBuffer   = 0;
	u32 m_UVBuffer       = 0;
	u32 m_IndexBuffer    = 0;

	std::vector<SubMesh> m_SubMeshes;

	// Note: The libraries the sub-meshes' materials come from, which also keeps them loaded
	std::vector<shared<Material>> m_Libraries;
};
