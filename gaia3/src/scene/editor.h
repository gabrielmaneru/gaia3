#pragma once

#include <core/types.h>
#include <math/quaternion.h>
#include <math/transform.h>
#include <math/vector.h>

#include <string_view>

class Entity;
class Level;

// Note: Selection shows no handles on the selected entity
enum class GizmoMode
{
	Selection,
	Translate,
	Rotate,
	Scale
};

// Note: Samples choose the editor through AppConfig::editor, nothing is created unless enabled is true
struct EditorConfig
{
	bool enabled = false;

	// Note: Floating mode (false) is currently only used for testing, it will serve custom non-sandbox projects
	bool docked = false;

	GizmoMode gizmoMode = GizmoMode::Selection;
};

// Note: Returns transform with only the part the mode edits taken from gizmoMatrix, the rest is kept as it was
Transform ApplyGizmoMatrix( GizmoMode mode, const Transform& transform, const Matrix44& gizmoMatrix );

class Editor
{
public:
	explicit Editor( const EditorConfig& config = {} );

	bool Init( std::string_view commonPath );
	void Draw();
	void Shutdown();

	// Note: While true, the engine's Input stops reporting the mouse or keyboard to the game
	bool WantsMouse() const;
	bool WantsKeyboard() const;

	// Note: When true, the game is shown inside the editor's Viewport window instead of the full window
	bool DisplaysRenderTarget() const { return m_Config.docked; }
	rect GetViewportSize() const { return m_ViewportSize; }

	Level*  GetLevel() const;
	Entity* GetSelectedEntity() const;
	void    SelectEntity( const Entity* entity );

	GizmoMode GetGizmoMode() const { return m_GizmoMode; }
	void      SetGizmoMode( GizmoMode mode ) { m_GizmoMode = mode; }

	void NewLevel();
	bool OpenLevel( std::string_view levelName );
	bool SaveLevel();
	bool SaveLevelAs( std::string_view fileName );

	Entity* CreateEntity( std::string_view name );
	Entity* CloneEntity( u64 id );
	bool    DeleteEntity( u64 id );

private:
	void DrawMenuBar();
	void DrawSaveAsPopup();
	void DrawRenamePopup();
	void DrawDockSpace();
	void DrawViewport();
	void DrawGizmo( Vec2 position, Vec2 size );
	void DrawHierarchy();
	void DrawInspector();
	void DrawEntityProperties( Entity& entity );

	EditorConfig m_Config;
	bool         m_bDockLayoutPending = true;
	bool         m_bViewportHovered   = false;
	bool         m_bGizmoInUse        = false;
	bool         m_bGameHoldsMouse    = false;
	bool         m_bSaveAsRequested   = false;
	bool         m_bRenameRequested   = false;
	char         m_SaveAsName[128]    = {};
	char         m_RenameName[128]    = {};
	float        m_DisplayScale       = 1.0f;
	rect         m_ViewportSize       = { 1, 1 };
	u64          m_SelectedEntityId   = 0;
	u64          m_RenameEntityId     = 0;
	GizmoMode    m_GizmoMode          = GizmoMode::Selection;

	// Note: Euler angles are kept between frames, reading them back from the rotation makes them jump while dragging
	u64  m_EulerEntityId = 0;
	Quat m_EulerSource;
	Vec3 m_EulerAngles;
};
