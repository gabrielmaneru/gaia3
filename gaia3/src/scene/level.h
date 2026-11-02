#pragma once

#include <core/types.h>
#include <scene/entity.h>
#include <system/resource.h>

#include <string_view>
#include <vector>

class Level : public Resource
{
	RTTI_DECLARE_RESOURCE( Level )
public:
	Level();
	explicit Level( std::string_view levelPath, ResourceState state = ResourceState::NotLoaded );
	~Level();

	bool LoadInternal() override;
	bool SaveAsInternal( std::string_view path ) override;

	Entity* CreateEntity( std::string_view entityName );
	bool    RemoveEntity( u64 id );
	void    ClearEntities();

	Entity*       FindEntity( u64 id );
	const Entity* FindEntity( u64 id ) const;

private:
	u64                         m_NextEntityId = 1;
	std::vector<unique<Entity>> m_Entities;
};
