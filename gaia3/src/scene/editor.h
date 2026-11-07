#pragma once

#include <core/types.h>
#include <system/rtti.h>

#include <string>
#include <string_view>
#include <vector>

class Entity;
class Level;

// Note: Samples choose the editor through AppConfig::editor, nothing is created unless enabled is true
struct EditorConfig
{
	bool enabled = false;
	bool docked  = false;
};

struct EditorProperty
{
	std::string Name;
	Json::Value Value;
};

class Editor
{
public:
	explicit Editor( const EditorConfig& config = {} );

	bool Init();
	void Draw();
	void Shutdown();

	// Note: While true, the engine's Input stops reporting the mouse or keyboard to the game
	bool WantsMouse() const;
	bool WantsKeyboard() const;

	// Note: When true, the game is shown inside the editor's Viewport window instead of the full window
	bool DisplaysRenderTarget() const { return m_Config.docked; }
	rect GetViewportSize() const { return m_ViewportSize; }

	const Level*  GetLevel() const;
	const Entity* GetSelectedEntity() const;
	void          SelectEntity( const Entity* entity );

	bool OpenLevel( std::string_view levelName );
	bool SaveLevel();
	bool SaveLevelAs( std::string_view fileName );

	// Note: Properties keep their declared order, and the value stays null if the type cannot be serialized
	static std::vector<EditorProperty> GetProperties( const void* instance, const rtti::TypeInfo& typeInfo );

private:
	void DrawMenuBar();
	void DrawSaveAsPopup();
	void DrawDockSpace();
	void DrawViewport();
	void DrawHierarchy();
	void DrawInspector() const;

	EditorConfig m_Config;
	bool         m_bDockLayoutPending = true;
	bool         m_bViewportHovered   = false;
	bool         m_bSaveAsRequested   = false;
	char         m_SaveAsName[128]    = {};
	float        m_DisplayScale       = 1.0f;
	rect         m_ViewportSize       = { 1, 1 };
	u64          m_SelectedEntityId   = 0;
};
