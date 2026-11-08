#pragma once

#include <gfx/rendererTypes.h>
#include <system/resource.h>

#include <vector>

class Material;

class Model : public Resource
{
	RTTI_DECLARE_RESOURCE( Model )
public:
	Model();
	explicit Model( std::string_view path, ResourceState state = ResourceState::NotLoaded );
	~Model();

	bool LoadInternal() override;
	bool UnloadInternal() override;

	// Note: Draws each sub-mesh with its own material, or every one with overrideMaterial when it is given
	void Draw( const DrawContext& context, const Material* overrideMaterial ) const;

private:
	struct SubMesh
	{
		u32              indexOffset = 0;
		u32              indexCount  = 0;
		shared<Material> material;
	};

	u32 m_VertexArray    = 0;
	u32 m_PositionBuffer = 0;
	u32 m_NormalBuffer   = 0;
	u32 m_UVBuffer       = 0;
	u32 m_IndexBuffer    = 0;

	std::vector<SubMesh> m_SubMeshes;
};
