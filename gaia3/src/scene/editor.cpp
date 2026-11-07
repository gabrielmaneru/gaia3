#include "gaia3_pch.h"
#include "editor.h"

#include <core/app.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <filesystem>

namespace
{
constexpr const char* kHierarchyWindow = "Hierarchy";
constexpr const char* kInspectorWindow = "Inspector";
constexpr const char* kViewportWindow  = "Viewport";
constexpr const char* kSaveAsPopup     = "Save Level As";

void DrawValue( const std::string& name, const Json::Value& value )
{
	if( value.isObject() )
	{
		if( ImGui::TreeNodeEx( name.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			for( const std::string& memberName : value.getMemberNames() )
			{
				DrawValue( memberName, value[memberName] );
			}

			ImGui::TreePop();
		}
	}
	else if( value.isArray() )
	{
		if( ImGui::TreeNodeEx( name.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			for( Json::ArrayIndex i = 0; i < value.size(); ++i )
			{
				DrawValue( std::to_string( i ), value[i] );
			}

			ImGui::TreePop();
		}
	}
	else if( value.isNull() )
	{
		ImGui::TextDisabled( "%s: unavailable", name.c_str() );
	}
	else if( value.isDouble() )
	{
		ImGui::Text( "%s: %.3f", name.c_str(), value.asDouble() );
	}
	else
	{
		ImGui::Text( "%s: %s", name.c_str(), value.asString().c_str() );
	}
}

void DrawProperties( const void* instance, const rtti::TypeInfo& typeInfo )
{
	for( const EditorProperty& property : Editor::GetProperties( instance, typeInfo ) )
	{
		DrawValue( property.Name, property.Value );
	}
}
} // namespace

Editor::Editor( const EditorConfig& config )
    : m_Config( config )
{
}

bool Editor::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();

	// Note: No ini file, so the layout always starts from EditorConfig
	io.IniFilename = nullptr;

	// Note: Matches the operating system's display scale, the default bitmap font only looks right at 100%
	m_DisplayScale = App::GetWindow().GetContentScale();
	if( m_DisplayScale != 1.0f )
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes( m_DisplayScale );
		style.FontScaleDpi = m_DisplayScale;
		io.Fonts->AddFontDefaultVector();
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

	if( m_Config.docked )
	{
		DrawMenuBar();
		DrawDockSpace();
		DrawViewport();
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
	// Note: The Viewport window is the game, so the mouse belongs to the game while hovering it
	return ImGui::GetIO().WantCaptureMouse && !m_bViewportHovered;
}

bool Editor::WantsKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

const Level* Editor::GetLevel() const
{
	return App::GetSession().GetCurrentLevel();
}

const Entity* Editor::GetSelectedEntity() const
{
	const Level* level = GetLevel();
	if( level == nullptr )
		return nullptr;

	return level->FindEntity( m_SelectedEntityId );
}

void Editor::SelectEntity( const Entity* entity )
{
	m_SelectedEntityId = entity != nullptr ? entity->GetId() : 0;
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

std::vector<EditorProperty> Editor::GetProperties( const void* instance, const rtti::TypeInfo& typeInfo )
{
	std::vector<EditorProperty> properties;

	const u8* rawInstance = reinterpret_cast<const u8*>( instance );
	for( size_t i = 0; i < typeInfo.PropertyCount; ++i )
	{
		const rtti::PropertyInfo& property = typeInfo.Properties[i];

		EditorProperty editorProperty;
		editorProperty.Name = property.Name;
		if( !property.Serialize( rawInstance + property.Offset, editorProperty.Value ) )
		{
			editorProperty.Value = Json::Value();
		}

		properties.push_back( std::move( editorProperty ) );
	}

	return properties;
}

void Editor::DrawMenuBar()
{
	if( ImGui::BeginMainMenuBar() )
	{
		if( ImGui::BeginMenu( "Level" ) )
		{
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
		if( ImGui::IsWindowAppearing() )
		{
			ImGui::SetKeyboardFocusHere();
		}

		const bool bConfirmed = ImGui::InputText( "Name", m_SaveAsName, sizeof( m_SaveAsName ), ImGuiInputTextFlags_EnterReturnsTrue );
		const bool bHasName   = m_SaveAsName[0] != '\0';

		ImGui::BeginDisabled( !bHasName );
		const bool bSavePressed = ImGui::Button( "Save" );
		ImGui::EndDisabled();

		ImGui::SameLine();
		const bool bCancelPressed = ImGui::Button( "Cancel" );

		if( ( bConfirmed || bSavePressed ) && bHasName )
		{
			SaveLevelAs( m_SaveAsName );
			ImGui::CloseCurrentPopup();
		}
		else if( bCancelPressed )
		{
			ImGui::CloseCurrentPopup();
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

	ImGui::DockSpaceOverViewport( dockspaceId, viewport );
}

void Editor::DrawViewport()
{
	m_bViewportHovered = false;

	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
	const bool bVisible = ImGui::Begin( kViewportWindow, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar );
	ImGui::PopStyleVar();

	if( bVisible )
	{
		const ImVec2 size = ImGui::GetContentRegionAvail();

		// Note: A collapsed panel can report no space, and a render target needs at least one pixel
		m_ViewportSize     = rect{ std::max( static_cast<int>( size.x ), 1 ), std::max( static_cast<int>( size.y ), 1 ) };
		m_bViewportHovered = ImGui::IsWindowHovered();

		// Note: The UVs are flipped because OpenGL textures start at the bottom left
		const u32 texture = App::GetRenderer().GetRenderTarget().GetColorTexture();
		ImGui::Image( static_cast<ImTextureID>( texture ), size, ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );
	}
	ImGui::End();
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
				ImGui::PopID();
			}
		}
	}
	ImGui::End();
}

void Editor::DrawInspector() const
{
	ImGui::SetNextWindowPos( ImVec2( 10.0f * m_DisplayScale, 320.0f * m_DisplayScale ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( ImVec2( 260.0f * m_DisplayScale, 500.0f * m_DisplayScale ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( kInspectorWindow ) )
	{
		const Entity* entity = GetSelectedEntity();
		if( entity == nullptr )
		{
			ImGui::TextDisabled( "No entity selected" );
		}
		else
		{
			DrawProperties( entity, Entity::StaticTypeInfo() );

			for( const shared<Component>& component : entity->GetComponents() )
			{
				if( component == nullptr )
					continue;

				const rtti::TypeInfo& typeInfo = component->GetTypeInfo();
				if( ImGui::CollapsingHeader( typeInfo.Name, ImGuiTreeNodeFlags_DefaultOpen ) )
				{
					DrawProperties( component.get(), typeInfo );
				}
			}
		}
	}
	ImGui::End();
}
