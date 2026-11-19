#include "levelEditor.h"

#include <core/app.h>
#include <editor/editorStyle.h>
#include <editor/inspector.h>
#include <math/quaternionTransforms.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cstring>
#include <filesystem>

using namespace EditorStyle;

namespace
{
constexpr const char* kHierarchyWindow = "Hierarchy";
constexpr const char* kInspectorWindow = "Inspector";
constexpr const char* kSaveAsPopup     = "Save Level As";
constexpr const char* kRenamePopup     = "Rename Entity";

bool IsSameRotation( const Quat& a, const Quat& b )
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
} // namespace

LevelEditor::LevelEditor( const EditorConfig& config )
    : EditorShell( config )
{
}

Level* LevelEditor::GetLevel() const
{
	return App::GetSession().GetCurrentLevel();
}

Entity* LevelEditor::GetSelectedEntity() const
{
	Level* level = GetLevel();
	if( level == nullptr )
		return nullptr;

	return level->FindEntity( m_SelectedEntityId );
}

void LevelEditor::SelectEntity( const Entity* entity )
{
	m_SelectedEntityId = entity != nullptr ? entity->GetId() : 0;
}

void LevelEditor::NewLevel()
{
	App::GetSession().NewLevel();
	SelectEntity( nullptr );
}

bool LevelEditor::OpenLevel( std::string_view levelName )
{
	if( !App::GetSession().OpenLevel( levelName ) )
		return false;

	// Note: Entity ids restart in every level, so the old selection could match an unrelated entity
	SelectEntity( nullptr );
	return true;
}

bool LevelEditor::SaveLevel()
{
	return App::GetSession().SaveCurrentLevel();
}

bool LevelEditor::SaveLevelAs( std::string_view fileName )
{
	std::filesystem::path path( fileName );
	if( !path.has_extension() )
	{
		path.replace_extension( ".lvl" );
	}

	return App::GetSession().SaveCurrentLevelAs( path.string() );
}

Entity* LevelEditor::CreateEntity( std::string_view name )
{
	Level* level = GetLevel();
	if( level == nullptr )
		return nullptr;

	Entity* entity = level->CreateEntity( name );
	SelectEntity( entity );
	return entity;
}

Entity* LevelEditor::CloneEntity( u64 id )
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

bool LevelEditor::DeleteEntity( u64 id )
{
	Level* level = GetLevel();
	return level != nullptr && level->RemoveEntity( id );
}

void LevelEditor::DrawMenus()
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
}

void LevelEditor::DrawPanels()
{
	DrawHierarchy();
	DrawInspector();

	// Note: Opened and drawn here rather than inside the menu bar, so the popup works in both layouts
	DrawSaveAsPopup();
}

void LevelEditor::BuildDockLayout( u32& viewportNode )
{
	ImGuiID hierarchyNode = 0;
	ImGuiID remainingNode = 0;
	ImGui::DockBuilderSplitNode( viewportNode, ImGuiDir_Left, 0.2f, &hierarchyNode, &remainingNode );

	ImGuiID inspectorNode = 0;
	ImGuiID centerNode    = 0;
	ImGui::DockBuilderSplitNode( remainingNode, ImGuiDir_Right, 0.25f, &inspectorNode, &centerNode );

	ImGui::DockBuilderDockWindow( kHierarchyWindow, hierarchyNode );
	ImGui::DockBuilderDockWindow( kInspectorWindow, inspectorNode );
	viewportNode = centerNode;
}

bool LevelEditor::GetGizmoTarget( Transform& outTransform ) const
{
	const Entity* entity = GetSelectedEntity();
	if( entity == nullptr )
		return false;

	outTransform = entity->GetTransform();
	return true;
}

void LevelEditor::SetGizmoTarget( const Transform& transform )
{
	if( Entity* entity = GetSelectedEntity() )
	{
		entity->SetTransform( transform );
	}
}

void LevelEditor::DrawSaveAsPopup()
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

void LevelEditor::DrawRenamePopup()
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

void LevelEditor::DrawHierarchy()
{
	const float displayScale = GetDisplayScale();
	ImGui::SetNextWindowPos( ImVec2( 10.0f * displayScale, 10.0f * displayScale ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( ImVec2( 260.0f * displayScale, 300.0f * displayScale ), ImGuiCond_FirstUseEver );

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

void LevelEditor::DrawInspector()
{
	const float displayScale = GetDisplayScale();
	ImGui::SetNextWindowPos( ImVec2( 10.0f * displayScale, 320.0f * displayScale ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( ImVec2( 260.0f * displayScale, 500.0f * displayScale ), ImGuiCond_FirstUseEver );

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

				PushGreyHeaders();
				const bool bOpen = ImGui::CollapsingHeader( component->GetTypeInfo().Name, ImGuiTreeNodeFlags_DefaultOpen );
				PopGreyHeaders();

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

void LevelEditor::DrawEntityProperties( Entity& entity )
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
