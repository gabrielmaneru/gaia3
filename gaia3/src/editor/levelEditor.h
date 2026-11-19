#pragma once

#include <editor/editorShell.h>
#include <math/quaternion.h>

#include <string_view>

class Entity;
class Level;

// Note: The editor for levels, the default one. Adds the Level menu, the Hierarchy of entities and the Inspector of
// the selected entity's transform and components to the shared EditorShell
class LevelEditor : public EditorShell
{
public:
	explicit LevelEditor( const EditorConfig& config = {} );

	Level*  GetLevel() const;
	Entity* GetSelectedEntity() const;
	void    SelectEntity( const Entity* entity );

	void NewLevel();
	bool OpenLevel( std::string_view levelName );
	bool SaveLevel();
	bool SaveLevelAs( std::string_view fileName );

	Entity* CreateEntity( std::string_view name );
	Entity* CloneEntity( u64 id );
	bool    DeleteEntity( u64 id );

protected:
	void DrawMenus() override;
	void DrawPanels() override;
	void BuildDockLayout( u32& viewportNode ) override;
	bool GetGizmoTarget( Transform& outTransform ) const override;
	void SetGizmoTarget( const Transform& transform ) override;

private:
	void DrawSaveAsPopup();
	void DrawRenamePopup();
	void DrawHierarchy();
	void DrawInspector();
	void DrawEntityProperties( Entity& entity );

	bool m_bSaveAsRequested = false;
	bool m_bRenameRequested = false;
	char m_SaveAsName[128]  = {};
	char m_RenameName[128]  = {};
	u64  m_SelectedEntityId = 0;
	u64  m_RenameEntityId   = 0;

	// Note: Euler angles are kept between frames, reading them back from the rotation makes them jump while dragging
	u64  m_EulerEntityId = 0;
	Quat m_EulerSource;
	Vec3 m_EulerAngles;
};
