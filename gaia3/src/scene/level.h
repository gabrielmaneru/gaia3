#pragma once

#include <core/types.h>
#include <scene/componentFactory.h>
#include <scene/entity.h>
#include <system/resource.h>

#include <string_view>
#include <vector>

class Level : public Resource
{
	RTTI_DECLARE_RESOURCE( Level )
public:
	Level();
	explicit Level( std::string_view path, ResourceState state = ResourceState::NotLoaded );
	~Level();

	void Update();

	bool LoadInternal() override;
	bool UnloadInternal() override;
	bool SaveAsInternal( std::string_view path ) override;

	Entity* CreateEntity( std::string_view entityName );
	Entity* CloneEntity( u64 id );
	bool    RemoveEntity( u64 id );
	void    ClearEntities();

	Entity*       FindEntity( u64 id );
	const Entity* FindEntity( u64 id ) const;

	const std::vector<unique<Entity>>& GetEntities() const { return m_Entities; }

	void                    RegisterComponent( const shared<Component>& component ) { m_ComponentFactory.RegisterComponent( component ); }
	void                    UnregisterComponent( const shared<Component>& component ) { m_ComponentFactory.UnregisterComponent( component ); }
	ComponentFactory&       GetComponentFactory() { return m_ComponentFactory; }
	const ComponentFactory& GetComponentFactory() const { return m_ComponentFactory; }

	template <typename T>
	std::vector<shared<T>> GetComponents()
	{
		return m_ComponentFactory.GetComponents<T>();
	}

private:
	std::string MakeCloneName( std::string_view sourceName ) const;

	ComponentFactory            m_ComponentFactory;
	u64                         m_NextEntityId = 1;
	std::vector<unique<Entity>> m_Entities;
};
