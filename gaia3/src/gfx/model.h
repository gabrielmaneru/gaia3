#pragma once

#include <system/resource.h>

class Model : public Resource
{
	RTTI_DECLARE_RESOURCE( Model )
public:
	Model();
	explicit Model( std::string_view path, ResourceState state = ResourceState::NotLoaded );
	~Model();

	bool LoadInternal() override;
	bool UnloadInternal() override;

	void Draw() const;

private:
	u32 m_VertexArray    = 0;
	u32 m_PositionBuffer = 0;
	u32 m_NormalBuffer   = 0;
	u32 m_UVBuffer       = 0;
	u32 m_IndexBuffer    = 0;
	u32 m_IndexCount     = 0;
};