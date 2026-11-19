#include "editorShell.h"

#include <components/cameraComponent.h>
#include <core/app.h>
#include <core/log.h>
#include <editor/editorStyle.h>
#include <math/mathHelpers.h>
#include <math/quaternionTransforms.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <cmath>
#include <filesystem>

using namespace EditorStyle;

namespace
{
constexpr float kFontSize = 15.0f;

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

struct GizmoSpaceItem
{
	const char* label;
	GizmoSpace  space;
};

constexpr GizmoSpaceItem kGizmoSpaceItems[] = {
    { "World", GizmoSpace::World },
    { "Local", GizmoSpace::Local },
};

struct DrawModeItem
{
	const char* label;
	DrawMode    mode;
};

constexpr DrawModeItem kDrawModeItems[] = {
    { "Lit", DrawMode::Lit },
    { "Normals", DrawMode::Normals },
    { "Shadows", DrawMode::Shadows },
    { "Roughness", DrawMode::Roughness },
    { "Metallic", DrawMode::Metallic },
};

ImGuizmo::MODE ToGizmoSpace( GizmoSpace space )
{
	switch( space )
	{
	case GizmoSpace::Local:
		return ImGuizmo::LOCAL;
	default:
		return ImGuizmo::WORLD;
	}
}

Vec3 GetMatrixAxis( const Matrix44& matrix, int axis )
{
	const Vec4& column = matrix.col[axis];
	return Vec3( column.x, column.y, column.z );
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

EditorShell::EditorShell( const EditorConfig& config )
    : m_Config( config ), m_GizmoMode( config.gizmoMode ), m_GizmoSpace( config.gizmoSpace )
{
}

bool EditorShell::Init( std::string_view commonPath )
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

void EditorShell::Draw()
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

	DrawPanels();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
}

void EditorShell::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool EditorShell::WantsMouse() const
{
	// Note: The Viewport window is the game, so the mouse belongs to the game while hovering it, unless a gizmo is dragged
	return m_bGizmoInUse || ( ImGui::GetIO().WantCaptureMouse && !m_bViewportHovered && !m_bGameHoldsMouse );
}

bool EditorShell::WantsKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool EditorShell::ProjectToViewport( const Vec3& worldPoint, Vec2 position, Vec2 size, Vec2& outScreenPoint )
{
	const RenderCamera* camera = App::GetRenderer().GetCamera();
	if( camera == nullptr )
		return false;

	const rect     viewportSize = { Max( static_cast<int>( size.x ), 1 ), Max( static_cast<int>( size.y ), 1 ) };
	const Matrix44 viewProjection = camera->GetProjectionMatrix( viewportSize ) * camera->GetViewMatrix();
	const Vec4     clip           = viewProjection * Vec4( worldPoint.x, worldPoint.y, worldPoint.z, 1.0f );
	if( clip.w <= 0.0001f )
		return false;

	// Note: Clip space has y up, screen pixels have y down
	const float x  = clip.x / clip.w;
	const float y  = clip.y / clip.w;
	outScreenPoint = Vec2( position.x + ( x * 0.5f + 0.5f ) * size.x, position.y + ( 0.5f - y * 0.5f ) * size.y );
	return true;
}

bool EditorShell::DrawNamePrompt( const char* confirmLabel, char* name, u64 nameSize )
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

void EditorShell::DrawMenuBar()
{
	if( ImGui::BeginMainMenuBar() )
	{
		DrawMenus();

		if( ImGui::BeginMenu( "Gizmo" ) )
		{
			for( const GizmoModeItem& item : kGizmoModeItems )
			{
				if( ImGui::MenuItem( item.label, nullptr, m_GizmoMode == item.mode ) )
				{
					SetGizmoMode( item.mode );
				}
			}

			ImGui::Separator();
			for( const GizmoSpaceItem& item : kGizmoSpaceItems )
			{
				if( ImGui::MenuItem( item.label, nullptr, m_GizmoSpace == item.space ) )
				{
					SetGizmoSpace( item.space );
				}
			}

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Settings" ) )
		{
			DrawRenderSettings();
			DrawShadowSettings();
			DrawCameraSettings();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void EditorShell::DrawRenderSettings()
{
	ImGui::SeparatorText( "Draw Mode" );

	Renderer& renderer = App::GetRenderer();
	for( const DrawModeItem& item : kDrawModeItems )
	{
		if( ImGui::MenuItem( item.label, nullptr, renderer.GetDrawMode() == item.mode ) )
		{
			renderer.SetDrawMode( item.mode );
		}
	}

	ImGui::Separator();
	if( ImGui::MenuItem( "Alpha Test", nullptr, renderer.IsAlphaTestEnabled() ) )
	{
		renderer.SetAlphaTestEnabled( !renderer.IsAlphaTestEnabled() );
	}
}

void EditorShell::DrawShadowSettings()
{
	ImGui::SeparatorText( "Shadows" );
	ImGui::PushItemWidth( 120.0f * m_DisplayScale );

	Renderer&      renderer = App::GetRenderer();
	ShadowSettings settings = renderer.GetShadowSettings();
	bool           bChanged = ImGui::Checkbox( "Sun shadows", &settings.bEnabled );

	// Note: Square maps, each step doubles the detail and quadruples the memory
	if( ImGui::BeginCombo( "Map size", std::to_string( settings.mapSize ).c_str() ) )
	{
		for( const u32 size : { 1024u, 2048u, 4096u } )
		{
			if( ImGui::Selectable( std::to_string( size ).c_str(), settings.mapSize == size ) )
			{
				settings.mapSize = size;
				bChanged         = true;
			}
		}
		ImGui::EndCombo();
	}

	bChanged |= ImGui::DragFloat( "Depth bias", &settings.depthBias, 0.00001f, 0.0f, 0.01f, "%.5f", ImGuiSliderFlags_AlwaysClamp );
	bChanged |= ImGui::DragFloat( "Normal bias", &settings.normalBias, 0.05f, 0.0f, 10.0f, "%.2f texels", ImGuiSliderFlags_AlwaysClamp );

	ImGui::PopItemWidth();

	if( ImGui::Button( "Reset##Shadows" ) )
	{
		settings = ShadowSettings();
		bChanged = true;
	}

	if( bChanged )
	{
		renderer.SetShadowSettings( settings );
	}
}

void EditorShell::DrawCameraSettings()
{
	ImGui::SeparatorText( "Camera" );
	ImGui::PushItemWidth( 120.0f * m_DisplayScale );

	float linearSpeed = FreeCameraComponent::GetLinearSpeed();
	if( ImGui::DragFloat( "Linear speed", &linearSpeed, 0.001f, 0.001f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp ) )
	{
		FreeCameraComponent::SetLinearSpeed( linearSpeed );
	}

	float angularSpeed = FreeCameraComponent::GetAngularSpeed();
	if( ImGui::DragFloat( "Angular speed", &angularSpeed, 0.005f, 0.01f, 5.0f, "%.2f deg", ImGuiSliderFlags_AlwaysClamp ) )
	{
		FreeCameraComponent::SetAngularSpeed( angularSpeed );
	}

	ImGui::PopItemWidth();

	if( ImGui::Button( "Reset" ) )
	{
		FreeCameraComponent::SetLinearSpeed( FreeCameraComponent::kDefaultLinearSpeed );
		FreeCameraComponent::SetAngularSpeed( FreeCameraComponent::kDefaultAngularSpeed );
	}
}

void EditorShell::DrawDockSpace()
{
	const ImGuiViewport* viewport    = ImGui::GetMainViewport();
	const ImGuiID        dockspaceId = ImGui::GetID( "EditorDockSpace" );

	if( m_bDockLayoutPending )
	{
		m_bDockLayoutPending = false;

		ImGui::DockBuilderRemoveNode( dockspaceId );
		ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
		ImGui::DockBuilderSetNodeSize( dockspaceId, viewport->WorkSize );

		u32 viewportNode = dockspaceId;
		BuildDockLayout( viewportNode );

		ImGui::DockBuilderDockWindow( kViewportWindow, viewportNode );
		ImGui::DockBuilderFinish( dockspaceId );
	}

	ImGui::DockSpaceOverViewport( dockspaceId, viewport, ImGuiDockNodeFlags_NoWindowMenuButton );
}

void EditorShell::DrawViewport()
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
		m_ViewportSize     = rect{ Max( static_cast<int>( size.x ), 1 ), Max( static_cast<int>( size.y ), 1 ) };
		m_bViewportHovered = ImGui::IsWindowHovered();

		// Note: The UVs are flipped because OpenGL textures start at the bottom left
		const u32 texture = App::GetRenderer().GetRenderTarget().GetColorTexture();
		ImGui::Image( static_cast<ImTextureID>( texture ), size, ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );

		DrawViewportOverlay( Vec2( position.x, position.y ), Vec2( size.x, size.y ) );

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

void EditorShell::DrawGizmo( Vec2 position, Vec2 size )
{
	m_bGizmoInUse = false;

	Transform           transform;
	const RenderCamera* camera = App::GetRenderer().GetCamera();
	if( m_GizmoMode == GizmoMode::Selection || camera == nullptr || !GetGizmoTarget( transform ) )
		return;

	const Matrix44 view       = camera->GetViewMatrix();
	const Matrix44 projection = camera->GetProjectionMatrix( m_ViewportSize );
	Matrix44       matrix     = transform.ToMatrix44();

	ImGuizmo::SetRect( position.x, position.y, size.x, size.y );
	if( ImGuizmo::Manipulate( &view.m00, &projection.m00, ToGizmoOperation( m_GizmoMode ), ToGizmoSpace( m_GizmoSpace ), &matrix.m00 ) )
	{
		SetGizmoTarget( ApplyGizmoMatrix( m_GizmoMode, transform, matrix ) );
	}

	m_bGizmoInUse = ImGuizmo::IsUsing();
}
