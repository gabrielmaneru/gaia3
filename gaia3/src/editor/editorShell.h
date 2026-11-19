#pragma once

#include <core/types.h>
#include <math/transform.h>
#include <math/vector.h>

#include <string_view>

// Note: Selection shows no handles on the selected entity
enum class GizmoMode
{
	Selection,
	Translate,
	Rotate,
	Scale
};

// Note: The axes the gizmo's handles follow. World uses the level's axes, Local the entity's own rotation, so
// translating along a local X moves the entity towards its own right. Scale always follows the entity's own axes
enum class GizmoSpace
{
	World,
	Local
};

// Note: Samples choose the editor through AppConfig::editor, nothing is created unless enabled is true
struct EditorConfig
{
	bool enabled = false;

	// Note: Floating mode (false) is currently only used for testing, it will serve custom non-sandbox projects
	bool docked = false;

	GizmoMode  gizmoMode  = GizmoMode::Selection;
	GizmoSpace gizmoSpace = GizmoSpace::World;
};

// Note: Returns transform with only the part the mode edits taken from gizmoMatrix, the rest is kept as it was
Transform ApplyGizmoMatrix( GizmoMode mode, const Transform& transform, const Matrix44& gizmoMatrix );

// Note: What every editor shares: ImGui with the editor style and font, the dock space, the Viewport showing the
// render target, the gizmo, and the Gizmo and Settings menus. Editors derive from it and fill in the hooks, like
// LevelEditor for levels or a sample's own tool
class EditorShell
{
public:
	static constexpr const char* kViewportWindow = "Viewport";

	explicit EditorShell( const EditorConfig& config = {} );
	virtual ~EditorShell() = default;

	bool Init( std::string_view commonPath );
	void Draw();
	void Shutdown();

	// Note: While true, the engine's Input stops reporting the mouse or keyboard to the game
	bool WantsMouse() const;
	bool WantsKeyboard() const;

	// Note: When true, the game is shown inside the editor's Viewport window instead of the full window
	bool DisplaysRenderTarget() const { return m_Config.docked; }
	rect GetViewportSize() const { return m_ViewportSize; }

	GizmoMode GetGizmoMode() const { return m_GizmoMode; }
	void      SetGizmoMode( GizmoMode mode ) { m_GizmoMode = mode; }

	GizmoSpace GetGizmoSpace() const { return m_GizmoSpace; }
	void       SetGizmoSpace( GizmoSpace space ) { m_GizmoSpace = space; }

protected:
	// Note: Menus of the main menu bar shown before the Gizmo and Settings menus, and anything they open, like popups
	virtual void DrawMenus() {}

	// Note: The editor's own windows, drawn after the Viewport
	virtual void DrawPanels() {}

	// Note: Docks the editor's windows once, when the layout is built. viewportNode starts as the whole dock space, an
	// editor splits panels off it and the Viewport takes what remains. Node ids are ImGuiID values
	virtual void BuildDockLayout( u32& viewportNode ) {}

	// Note: The transform the gizmo moves, false when there is nothing to move
	virtual bool GetGizmoTarget( Transform& outTransform ) const { return false; }
	virtual void SetGizmoTarget( const Transform& transform ) {}

	// Note: Draws over the rendered image, before the gizmo. position and size are the image's area in screen pixels,
	// ImGui's window draw list paints there
	virtual void DrawViewportOverlay( Vec2 position, Vec2 size ) {}

	float GetDisplayScale() const { return m_DisplayScale; }

	// Note: Where a world point lands in the image drawn at position and size, false when it is behind the camera
	static bool ProjectToViewport( const Vec3& worldPoint, Vec2 position, Vec2 size, Vec2& outScreenPoint );

	// Note: Draws the contents of a modal popup that asks for a name, and returns true once it is confirmed
	static bool DrawNamePrompt( const char* confirmLabel, char* name, u64 nameSize );

private:
	void DrawMenuBar();
	void DrawRenderSettings();
	void DrawShadowSettings();
	void DrawCameraSettings();
	void DrawDockSpace();
	void DrawViewport();
	void DrawGizmo( Vec2 position, Vec2 size );

	EditorConfig m_Config;
	bool         m_bDockLayoutPending = true;
	bool         m_bViewportHovered   = false;
	bool         m_bGizmoInUse        = false;
	bool         m_bGameHoldsMouse    = false;
	float        m_DisplayScale       = 1.0f;
	rect         m_ViewportSize       = { 1, 1 };
	GizmoMode    m_GizmoMode          = GizmoMode::Selection;
	GizmoSpace   m_GizmoSpace         = GizmoSpace::World;
};
