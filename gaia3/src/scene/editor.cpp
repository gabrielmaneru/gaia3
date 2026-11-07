#include "gaia3_pch.h"
#include "editor.h"

#include <core/app.h>
#include <scene/inspector.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <ImGuizmo.h>

#include <cmath>
#include <cstring>
#include <filesystem>

namespace
{
constexpr const char* kHierarchyWindow = "Hierarchy";
constexpr const char* kInspectorWindow = "Inspector";
constexpr const char* kViewportWindow  = "Viewport";
constexpr const char* kSaveAsPopup     = "Save Level As";
constexpr const char* kRenamePopup     = "Rename Entity";

constexpr ImGuiPopupFlags kEmptySpaceContextMenu = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;

constexpr float kFontSize = 15.0f;

constexpr ImVec4 RgbColor( u32 hex, float alpha = 1.0f )
{
	return ImVec4( ( ( hex >> 16 ) & 0xFF ) / 255.0f, ( ( hex >> 8 ) & 0xFF ) / 255.0f, ( hex & 0xFF ) / 255.0f, alpha );
}

constexpr ImVec4 kColorPanel         = RgbColor( 0x333333 );
constexpr ImVec4 kColorStrip         = RgbColor( 0x232323 );
constexpr ImVec4 kColorField         = RgbColor( 0x262626 );
constexpr ImVec4 kColorFieldHovered  = RgbColor( 0x2E2E2E );
constexpr ImVec4 kColorControl       = RgbColor( 0x454545 );
constexpr ImVec4 kColorControlHover  = RgbColor( 0x525252 );
constexpr ImVec4 kColorControlActive = RgbColor( 0x5E5E5E );
constexpr ImVec4 kColorBorder        = RgbColor( 0x1A1A1A );
constexpr ImVec4 kColorAccent        = RgbColor( 0x2C5D87 );
constexpr ImVec4 kColorAccentHover   = RgbColor( 0x3A76AB );
constexpr ImVec4 kColorAccentBright  = RgbColor( 0x4C9BE0 );
constexpr ImVec4 kColorText          = RgbColor( 0xD4D4D4 );
constexpr ImVec4 kColorTextDisabled  = RgbColor( 0x7E7E7E );
constexpr ImVec4 kColorInvisible     = RgbColor( 0x000000, 0.0f );

// Note: Neutral greys with one blue accent, like the Unity and Unreal editors. The sizes are for a 100% display, Init scales them
void ApplyEditorStyle()
{
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding      = ImVec2( 8.0f, 8.0f );
	style.WindowRounding     = 0.0f;
	style.WindowBorderSize   = 1.0f;
	style.PopupRounding      = 3.0f;
	style.PopupBorderSize    = 1.0f;
	style.FramePadding       = ImVec2( 6.0f, 3.0f );
	style.FrameRounding      = 2.0f;
	style.FrameBorderSize    = 1.0f;
	style.ItemSpacing        = ImVec2( 6.0f, 4.0f );
	style.ItemInnerSpacing   = ImVec2( 4.0f, 4.0f );
	style.IndentSpacing      = 14.0f;
	style.ScrollbarSize      = 12.0f;
	style.ScrollbarRounding  = 2.0f;
	style.GrabRounding       = 2.0f;
	style.TabRounding        = 2.0f;
	style.TabBarOverlineSize = 2.0f;

	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text]         = kColorText;
	colors[ImGuiCol_TextDisabled] = kColorTextDisabled;
	colors[ImGuiCol_WindowBg]     = kColorPanel;
	colors[ImGuiCol_PopupBg]      = kColorStrip;
	colors[ImGuiCol_Border]       = kColorBorder;

	colors[ImGuiCol_FrameBg]        = kColorField;
	colors[ImGuiCol_FrameBgHovered] = kColorFieldHovered;
	colors[ImGuiCol_FrameBgActive]  = kColorField;

	colors[ImGuiCol_TitleBg]          = kColorStrip;
	colors[ImGuiCol_TitleBgActive]    = kColorStrip;
	colors[ImGuiCol_TitleBgCollapsed] = kColorStrip;
	colors[ImGuiCol_MenuBarBg]        = kColorStrip;

	colors[ImGuiCol_ScrollbarBg]          = kColorField;
	colors[ImGuiCol_ScrollbarGrab]        = kColorControl;
	colors[ImGuiCol_ScrollbarGrabHovered] = kColorControlHover;
	colors[ImGuiCol_ScrollbarGrabActive]  = kColorControlActive;

	colors[ImGuiCol_CheckMark]        = kColorAccentBright;
	colors[ImGuiCol_SliderGrab]       = kColorControlActive;
	colors[ImGuiCol_SliderGrabActive] = kColorAccentBright;

	colors[ImGuiCol_Button]        = kColorControl;
	colors[ImGuiCol_ButtonHovered] = kColorControlHover;
	colors[ImGuiCol_ButtonActive]  = kColorControlActive;

	colors[ImGuiCol_Header]        = kColorAccent;
	colors[ImGuiCol_HeaderHovered] = kColorAccentHover;
	colors[ImGuiCol_HeaderActive]  = kColorAccentHover;

	colors[ImGuiCol_Separator]        = kColorBorder;
	colors[ImGuiCol_SeparatorHovered] = kColorAccentHover;
	colors[ImGuiCol_SeparatorActive]  = kColorAccentBright;

	colors[ImGuiCol_ResizeGrip]        = kColorInvisible;
	colors[ImGuiCol_ResizeGripHovered] = kColorAccentHover;
	colors[ImGuiCol_ResizeGripActive]  = kColorAccentBright;

	colors[ImGuiCol_Tab]                       = kColorStrip;
	colors[ImGuiCol_TabHovered]                = kColorControl;
	colors[ImGuiCol_TabSelected]               = kColorPanel;
	colors[ImGuiCol_TabSelectedOverline]       = kColorAccentBright;
	colors[ImGuiCol_TabDimmed]                 = kColorStrip;
	colors[ImGuiCol_TabDimmedSelected]         = kColorPanel;
	colors[ImGuiCol_TabDimmedSelectedOverline] = kColorInvisible;

	colors[ImGuiCol_DockingPreview] = RgbColor( 0x4C9BE0, 0.5f );
	colors[ImGuiCol_DockingEmptyBg] = kColorStrip;
	colors[ImGuiCol_TextSelectedBg] = RgbColor( 0x4C9BE0, 0.35f );
	colors[ImGuiCol_NavCursor]      = kColorAccentBright;
}

void LoadEditorFont( std::string_view commonPath )
{
	const std::filesystem::path fontPath = std::filesystem::path( commonPath ) / "fonts" / "CascadiaMono-Regular.ttf";

	// Note: ImGui asserts when a font file is missing, so it is checked first
	ImFontAtlas* fonts = ImGui::GetIO().Fonts;
	if( !std::filesystem::is_regular_file( fontPath ) || fonts->AddFontFromFileTTF( fontPath.string().c_str(), kFontSize ) == nullptr )
	{
		GAIA_WARN( "Editor: failed to load the font {}, using ImGui's default one", fontPath.string() );
		fonts->AddFontDefaultVector();
	}
}

struct GizmoModeItem
{
	const char* label;
	GizmoMode   mode;
};

constexpr GizmoModeItem kGizmoModeItems[] = {
	{ "Selection", GizmoMode::Selection },
	{ "Translate", GizmoMode::Translate },
	{ "Rotate", GizmoMode::Rotate },
	{ "Scale", GizmoMode::Scale },
};

ImGuizmo::OPERATION ToGizmoOperation( GizmoMode mode )
{
	switch( mode )
	{
	case GizmoMode::Rotate:
		return ImGuizmo::ROTATE;
	case GizmoMode::Scale:
		return ImGuizmo::SCALE;
	default:
		return ImGuizmo::TRANSLATE;
	}
}

Vec3 GetMatrixAxis( const Matrix44& matrix, int axis )
{
	const Vec4& column = matrix.col[axis];
	return Vec3( column.x, column.y, column.z );
}

// Note: Draws the contents of a modal popup that asks for a name, and returns true once it is confirmed
bool DrawNamePrompt( const char* confirmLabel, char* name, size_t nameSize )
{
	if( ImGui::IsWindowAppearing() )
	{
		ImGui::SetKeyboardFocusHere();
	}

	const bool bConfirmed = ImGui::InputText( "Name", name, nameSize, ImGuiInputTextFlags_EnterReturnsTrue );
	const bool bHasName   = name[0] != '\0';

	ImGui::BeginDisabled( !bHasName );
	const bool bConfirmPressed = ImGui::Button( confirmLabel );
	ImGui::EndDisabled();

	ImGui::SameLine();
	const bool bCancelPressed = ImGui::Button( "Cancel" );

	if( ( bConfirmed || bConfirmPressed ) && bHasName )
	{
		ImGui::CloseCurrentPopup();
		return true;
	}

	if( bCancelPressed )
	{
		ImGui::CloseCurrentPopup();
	}

	return false;
}

bool IsSameRotation( const Quat& a, const Quat& b )
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
} // namespace

Transform ApplyGizmoMatrix( GizmoMode mode, const Transform& transform, const Matrix44& gizmoMatrix )
{
	const Vec3 axisX = GetMatrixAxis( gizmoMatrix, 0 );
	const Vec3 axisY = GetMatrixAxis( gizmoMatrix, 1 );
	const Vec3 axisZ = GetMatrixAxis( gizmoMatrix, 2 );
	const Vec3 scale = transform.scale;

	Transform result = transform;
	switch( mode )
	{
	case GizmoMode::Selection:
		break;
	case GizmoMode::Translate:
		result.position = GetMatrixAxis( gizmoMatrix, 3 );
		break;
	case GizmoMode::Rotate:
		// Note: A zero scale leaves no axis to read the rotation from
		if( fabsf( scale.x ) > eps_6 && fabsf( scale.y ) > eps_6 && fabsf( scale.z ) > eps_6 )
		{
			result.rotation = QuatFromMatrix33( Matrix33( axisX * Scalar( 1.0f / scale.x ),
			                                              axisY * Scalar( 1.0f / scale.y ),
			                                              axisZ * Scalar( 1.0f / scale.z ) ) );
		}
		break;
	case GizmoMode::Scale:
		// Note: The matrix only tells the size of each axis, so a mirrored axis keeps its sign from the entity
		result.scale = Vec3( copysignf( sqrtf( axisX.LengthSq() ), scale.x ),
		                     copysignf( sqrtf( axisY.LengthSq() ), scale.y ),
		                     copysignf( sqrtf( axisZ.LengthSq() ), scale.z ) );
		break;
	}

	return result;
}

Editor::Editor( const EditorConfig& config )
    : m_Config( config ), m_GizmoMode( config.gizmoMode )
{
}

bool Editor::Init( std::string_view commonPath )
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ApplyEditorStyle();

	ImGuiIO& io = ImGui::GetIO();

	// Note: No ini file, so the layout always starts from EditorConfig
	io.IniFilename = nullptr;

	LoadEditorFont( commonPath );

	// Note: Matches the operating system's display scale
	m_DisplayScale = App::GetWindow().GetContentScale();
	if( m_DisplayScale != 1.0f )
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes( m_DisplayScale );
		style.FontScaleDpi = m_DisplayScale;

		ImGuizmo::Style& gizmoStyle = ImGuizmo::GetStyle();
		gizmoStyle.TranslationLineThickness *= m_DisplayScale;
		gizmoStyle.TranslationLineArrowSize *= m_DisplayScale;
		gizmoStyle.RotationLineThickness *= m_DisplayScale;
		gizmoStyle.RotationOuterLineThickness *= m_DisplayScale;
		gizmoStyle.ScaleLineThickness *= m_DisplayScale;
		gizmoStyle.ScaleLineCircleSize *= m_DisplayScale;
		gizmoStyle.CenterCircleSize *= m_DisplayScale;
	}

	if( m_Config.docked )
	{
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	// Note: Needs Input::Init to have run first, ImGui then forwards the GLFW callbacks to it
	if( !ImGui_ImplGlfw_InitForOpenGL( App::GetWindow().GetWindowHandle(), true ) )
	{
		GAIA_ERROR( "Editor: failed to initialize the ImGui GLFW backend" );
		ImGui::DestroyContext();
		return false;
	}

	if( !ImGui_ImplOpenGL3_Init( "#version 330 core" ) )
	{
		GAIA_ERROR( "Editor: failed to initialize the ImGui OpenGL backend" );
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		return false;
	}

	m_ViewportSize = App::GetWindow().GetCanvas();

	GAIA_INFO( "Editor: ImGui {}, {} layout, scale {}", ImGui::GetVersion(), m_Config.docked ? "docked" : "floating", m_DisplayScale );
	return true;
}

void Editor::Draw()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	if( m_bGameHoldsMouse && !ImGui::IsAnyMouseDown() )
	{
		m_bGameHoldsMouse = false;
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	if( m_Config.docked )
	{
		DrawMenuBar();
		DrawDockSpace();
		DrawViewport();
	}
	else
	{
		// Note: The game fills the window without a Viewport panel, so the gizmo covers the whole window
		const rect canvas = App::GetWindow().GetCanvas();
		m_ViewportSize    = canvas;
		DrawGizmo( Vec2( 0.0f, 0.0f ), Vec2( static_cast<float>( canvas.x ), static_cast<float>( canvas.y ) ) );
	}

	DrawHierarchy();
	DrawInspector();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
}

void Editor::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool Editor::WantsMouse() const
{
	// Note: The Viewport window is the game, so the mouse belongs to the game while hovering it, unless a gizmo is dragged
	return m_bGizmoInUse || ( ImGui::GetIO().WantCaptureMouse && !m_bViewportHovered && !m_bGameHoldsMouse );
}

bool Editor::WantsKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

Level* Editor::GetLevel() const
{
	return App::GetSession().GetCurrentLevel();
}

Entity* Editor::GetSelectedEntity() const
{
	Level* level = GetLevel();
	if( level == nullptr )
		return nullptr;

	return level->FindEntity( m_SelectedEntityId );
}

void Editor::SelectEntity( const Entity* entity )
{
	m_SelectedEntityId = entity != nullptr ? entity->GetId() : 0;
}

void Editor::NewLevel()
{
	App::GetSession().NewLevel();
	SelectEntity( nullptr );
}
bool Editor::OpenLevel( std::string_view levelName )
{
	if( !App::GetSession().OpenLevel( levelName ) )
		return false;

	// Note: Entity ids restart in every level, so the old selection could match an unrelated entity
	SelectEntity( nullptr );
	return true;
}

bool Editor::SaveLevel()
{
	return App::GetSession().SaveCurrentLevel();
}

bool Editor::SaveLevelAs( std::string_view fileName )
{
	std::filesystem::path path( fileName );
	if( !path.has_extension() )
	{
		path.replace_extension( ".lvl" );
	}

	return App::GetSession().SaveCurrentLevelAs( path.string() );
}

Entity* Editor::CreateEntity( std::string_view name )
{
	Level* level = GetLevel();
	if( level == nullptr )
		return nullptr;

	Entity* entity = level->CreateEntity( name );
	SelectEntity( entity );
	return entity;
}

Entity* Editor::CloneEntity( u64 id )
{
	Level* level = GetLevel();
	if( level == nullptr )
		return nullptr;

	Entity* clone = level->CloneEntity( id );
	if( clone != nullptr )
	{
		SelectEntity( clone );
	}

	return clone;
}

bool Editor::DeleteEntity( u64 id )
{
	Level* level = GetLevel();
	return level != nullptr && level->RemoveEntity( id );
}
void Editor::DrawMenuBar()
{
	if( ImGui::BeginMainMenuBar() )
	{
		if( ImGui::BeginMenu( "Level" ) )
		{
			if( ImGui::MenuItem( "New" ) )
			{
				NewLevel();
			}

			const std::vector<std::string> levelNames = App::GetSession().GetResourceManager().GetResourceNames<Level>();
			if( ImGui::BeginMenu( "Open", !levelNames.empty() ) )
			{
				for( const std::string& levelName : levelNames )
				{
					if( ImGui::MenuItem( levelName.c_str() ) )
					{
						OpenLevel( levelName );
					}
				}

				ImGui::EndMenu();
			}

			const Level* level = GetLevel();
			if( ImGui::MenuItem( "Save", nullptr, false, level != nullptr ) )
			{
				if( level->IsNew() )
				{
					m_bSaveAsRequested = true;
				}
				else
				{
					SaveLevel();
				}
			}

			if( ImGui::MenuItem( "Save As...", nullptr, false, level != nullptr ) )
			{
				m_bSaveAsRequested = true;
			}

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Gizmo" ) )
		{
			for( const GizmoModeItem& item : kGizmoModeItems )
			{
				if( ImGui::MenuItem( item.label, nullptr, m_GizmoMode == item.mode ) )
				{
					SetGizmoMode( item.mode );
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	DrawSaveAsPopup();
}

void Editor::DrawSaveAsPopup()
{
	if( m_bSaveAsRequested )
	{
		m_bSaveAsRequested = false;
		m_SaveAsName[0]    = '\0';
		ImGui::OpenPopup( kSaveAsPopup );
	}

	if( ImGui::BeginPopupModal( kSaveAsPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( DrawNamePrompt( "Save", m_SaveAsName, sizeof( m_SaveAsName ) ) )
		{
			SaveLevelAs( m_SaveAsName );
		}

		ImGui::EndPopup();
	}
}

void Editor::DrawRenamePopup()
{
	if( m_bRenameRequested )
	{
		m_bRenameRequested = false;
		ImGui::OpenPopup( kRenamePopup );
	}

	if( ImGui::BeginPopupModal( kRenamePopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( DrawNamePrompt( "Rename", m_RenameName, sizeof( m_RenameName ) ) )
		{
			Entity* entity = GetLevel() != nullptr ? GetLevel()->FindEntity( m_RenameEntityId ) : nullptr;
			if( entity != nullptr )
			{
				entity->SetName( m_RenameName );
			}
		}

		ImGui::EndPopup();
	}
}
void Editor::DrawDockSpace()
{
	const ImGuiViewport* viewport    = ImGui::GetMainViewport();
	const ImGuiID        dockspaceId = ImGui::GetID( "EditorDockSpace" );

	if( m_bDockLayoutPending )
	{
		m_bDockLayoutPending = false;

		ImGui::DockBuilderRemoveNode( dockspaceId );
		ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
		ImGui::DockBuilderSetNodeSize( dockspaceId, viewport->WorkSize );

		ImGuiID hierarchyNode = 0;
		ImGuiID remainingNode = 0;
		ImGui::DockBuilderSplitNode( dockspaceId, ImGuiDir_Left, 0.2f, &hierarchyNode, &remainingNode );

		ImGuiID inspectorNode = 0;
		ImGuiID viewportNode  = 0;
		ImGui::DockBuilderSplitNode( remainingNode, ImGuiDir_Right, 0.25f, &inspectorNode, &viewportNode );

		ImGui::DockBuilderDockWindow( kHierarchyWindow, hierarchyNode );
		ImGui::DockBuilderDockWindow( kViewportWindow, viewportNode );
		ImGui::DockBuilderDockWindow( kInspectorWindow, inspectorNode );
		ImGui::DockBuilderFinish( dockspaceId );
	}

	ImGui::DockSpaceOverViewport( dockspaceId, viewport, ImGuiDockNodeFlags_NoWindowMenuButton );
}

void Editor::DrawViewport()
{
	m_bViewportHovered = false;

	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
	const bool bVisible = ImGui::Begin( kViewportWindow, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar );
	ImGui::PopStyleVar();

	if( bVisible )
	{
		const ImVec2 position = ImGui::GetCursorScreenPos();
		const ImVec2 size     = ImGui::GetContentRegionAvail();

		// Note: A collapsed panel can report no space, and a render target needs at least one pixel
		m_ViewportSize     = rect{ std::max( static_cast<int>( size.x ), 1 ), std::max( static_cast<int>( size.y ), 1 ) };
		m_bViewportHovered = ImGui::IsWindowHovered();

		// Note: The UVs are flipped because OpenGL textures start at the bottom left
		const u32 texture = App::GetRenderer().GetRenderTarget().GetColorTexture();
		ImGui::Image( static_cast<ImTextureID>( texture ), size, ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );

		ImGuizmo::SetDrawlist();
		DrawGizmo( Vec2( position.x, position.y ), Vec2( size.x, size.y ) );

		// Note: A press that starts in the Viewport stays with the game until released, NoMouse keeps ImGui from
		// reacting to the mouse on its way, otherwise releasing over a panel would open its context menu
		const bool bPressed = ImGui::IsMouseClicked( ImGuiMouseButton_Left ) || ImGui::IsMouseClicked( ImGuiMouseButton_Right ) || ImGui::IsMouseClicked( ImGuiMouseButton_Middle );
		if( bPressed && m_bViewportHovered && !m_bGizmoInUse )
		{
			m_bGameHoldsMouse = true;
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}
	}
	ImGui::End();
}

void Editor::DrawGizmo( Vec2 position, Vec2 size )
{
	m_bGizmoInUse = false;

	Entity*             entity = GetSelectedEntity();
	const RenderCamera* camera = App::GetRenderer().GetCamera();
	if( m_GizmoMode == GizmoMode::Selection || entity == nullptr || camera == nullptr )
		return;

	const Matrix44 view       = camera->GetViewMatrix();
	const Matrix44 projection = camera->GetProjectionMatrix( m_ViewportSize );

	const Transform transform = entity->GetTransform();
	Matrix44        matrix    = transform.ToMatrix44();

	ImGuizmo::SetRect( position.x, position.y, size.x, size.y );
	if( ImGuizmo::Manipulate( &view.m00, &projection.m00, ToGizmoOperation( m_GizmoMode ), ImGuizmo::WORLD, &matrix.m00 ) )
	{
		entity->SetTransform( ApplyGizmoMatrix( m_GizmoMode, transform, matrix ) );
	}

	m_bGizmoInUse = ImGuizmo::IsUsing();
}

void Editor::DrawHierarchy()
{
	ImGui::SetNextWindowPos( ImVec2( 10.0f * m_DisplayScale, 10.0f * m_DisplayScale ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( ImVec2( 260.0f * m_DisplayScale, 300.0f * m_DisplayScale ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( kHierarchyWindow ) )
	{
		const Level* level = GetLevel();
		if( level != nullptr )
		{
			ImGui::Text( "Level: %s", level->IsNew() ? "New" : std::string( level->GetName() ).c_str() );
			ImGui::Separator();

			// Note: Changes to the entity list wait until the loop ends, they would invalidate it
			u64 entityToClone  = 0;
			u64 entityToDelete = 0;

			const Entity* selectedEntity = GetSelectedEntity();
			for( const unique<Entity>& entity : level->GetEntities() )
			{
				if( entity == nullptr )
					continue;

				ImGui::PushID( static_cast<int>( entity->GetId() ) );
				if( ImGui::Selectable( std::string( entity->GetName() ).c_str(), entity.get() == selectedEntity ) )
				{
					SelectEntity( entity.get() );
				}

				if( ImGui::BeginPopupContextItem() )
				{
					SelectEntity( entity.get() );

					if( ImGui::MenuItem( "Rename..." ) )
					{
						m_RenameEntityId   = entity->GetId();
						m_bRenameRequested = true;

						const std::string name( entity->GetName() );
						std::strncpy( m_RenameName, name.c_str(), sizeof( m_RenameName ) - 1 );
					}

					if( ImGui::MenuItem( "Clone" ) )
					{
						entityToClone = entity->GetId();
					}

					if( ImGui::MenuItem( "Delete" ) )
					{
						entityToDelete = entity->GetId();
					}

					ImGui::EndPopup();
				}
				ImGui::PopID();
			}

			if( ImGui::BeginPopupContextWindow( "HierarchyContextMenu", kEmptySpaceContextMenu ) )
			{
				if( ImGui::MenuItem( "Create Entity" ) )
				{
					CreateEntity( "Entity" );
				}

				ImGui::EndPopup();
			}

			if( entityToClone != 0 )
			{
				CloneEntity( entityToClone );
			}

			if( entityToDelete != 0 )
			{
				DeleteEntity( entityToDelete );
			}
		}

		DrawRenamePopup();
	}
	ImGui::End();
}

void Editor::DrawInspector()
{
	ImGui::SetNextWindowPos( ImVec2( 10.0f * m_DisplayScale, 320.0f * m_DisplayScale ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( ImVec2( 260.0f * m_DisplayScale, 500.0f * m_DisplayScale ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( kInspectorWindow ) )
	{
		Entity* entity = GetSelectedEntity();
		if( entity == nullptr )
		{
			ImGui::TextDisabled( "No entity selected" );
		}
		else
		{
			DrawEntityProperties( *entity );

			// Note: Removing waits until the loop ends, it would invalidate the component list
			Component* componentToRemove = nullptr;

			int componentIndex = 0;
			for( const shared<Component>& component : entity->GetComponents() )
			{
				if( component == nullptr )
					continue;

				ImGui::PushID( componentIndex++ );

				// Note: Headers share their color with selected rows, which should be the only blue in the panels
				ImGui::PushStyleColor( ImGuiCol_Header, kColorControl );
				ImGui::PushStyleColor( ImGuiCol_HeaderHovered, kColorControlHover );
				ImGui::PushStyleColor( ImGuiCol_HeaderActive, kColorControlActive );
				const bool bOpen = ImGui::CollapsingHeader( component->GetTypeInfo().Name, ImGuiTreeNodeFlags_DefaultOpen );
				ImGui::PopStyleColor( 3 );

				if( ImGui::BeginPopupContextItem() )
				{
					if( ImGui::MenuItem( "Remove Component" ) )
					{
						componentToRemove = component.get();
					}

					ImGui::EndPopup();
				}

				if( bOpen )
				{
					component->DrawInspector();
				}

				ImGui::PopID();
			}

			if( ImGui::BeginPopupContextWindow( "InspectorContextMenu", kEmptySpaceContextMenu ) )
			{
				if( ImGui::BeginMenu( "Add Component" ) )
				{
					for( const std::string& typeName : ComponentFactory::GetRegisteredTypeNames() )
					{
						if( ImGui::MenuItem( typeName.c_str(), nullptr, false, !entity->HasComponent( typeName ) ) )
						{
							entity->AddComponentByName( typeName );
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			if( componentToRemove != nullptr )
			{
				entity->RemoveComponent( componentToRemove );
			}
		}
	}
	ImGui::End();
}

void Editor::DrawEntityProperties( Entity& entity )
{
	std::string name( entity.GetName() );
	if( Inspector::EditString( "Name", name ) )
	{
		entity.SetName( name );
	}

	ImGui::Separator();

	Transform transform = entity.GetTransform();

	if( entity.GetId() != m_EulerEntityId || !IsSameRotation( transform.rotation, m_EulerSource ) )
	{
		m_EulerEntityId = entity.GetId();
		m_EulerSource   = transform.rotation;
		m_EulerAngles   = EulerFromQuat( transform.rotation );
	}

	bool bTransformChanged = Inspector::EditVec3( "Position", transform.position );

	if( Inspector::EditVec3( "Rotation", m_EulerAngles, 0.5f ) )
	{
		transform.rotation = QuatFromEuler( m_EulerAngles );
		m_EulerSource      = transform.rotation;
		bTransformChanged  = true;
	}

	bTransformChanged |= Inspector::EditVec3( "Scale", transform.scale, 0.01f );

	if( bTransformChanged )
	{
		entity.SetTransform( transform );
	}
}