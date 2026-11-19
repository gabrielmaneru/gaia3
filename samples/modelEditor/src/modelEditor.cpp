#include "modelEditor.h"

#include "components/previewModelComponent.h"
#include "modelOperations.h"

#include <components/cameraComponent.h>
#include <components/lightComponent.h>
#include <components/modelComponent.h>
#include <core/app.h>
#include <editor/editorStyle.h>
#include <editor/inspector.h>
#include <gfx/material.h>
#include <gfx/model.h>
#include <math/mathHelpers.h>
#include <math/matrix.h>
#include <math/quaternionTransforms.h>
#include <math/vectorTransforms.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

using namespace EditorStyle;

namespace
{
constexpr const char* kModelWindow      = "Model";
constexpr const char* kInspectorWindow  = "Inspector";
constexpr const char* kStatisticsWindow = "Statistics";
constexpr const char* kImportPopup      = "Import Model";

// Note: The engine's plane, which faces up, stretched under the model
constexpr const char* kGroundModel = "plane";

// Note: The sun comes from high up and to one side, so shapes read well
const Vec3 kSunDirection = Vec3( -0.4f, -0.8f, -0.6f ).Normalized();

// Note: The model's index range of a mesh, or of one sub-mesh when subMeshIndex is not -1
AABB GetSelectionBoundingVolume( const ModelData& data, int meshIndex, int subMeshIndex )
{
	if( meshIndex < 0 )
		return AABB();

	return subMeshIndex < 0 ? data.GetMeshBoundingVolume( meshIndex ) : data.GetSubMeshBoundingVolume( meshIndex, subMeshIndex );
}

u64 GetMeshTriangleCount( const MeshData& mesh )
{
	u64 indexCount = 0;
	for( const SubMeshData& subMesh : mesh.subMeshes )
	{
		indexCount += subMesh.indexCount;
	}
	return indexCount / 3;
}

const char* GetSlotLabel( const std::string& slot )
{
	return slot.empty() ? "(no material)" : slot.c_str();
}

void GetBoxCorners( const AABB& box, Vec3 outCorners[8] )
{
	outCorners[0] = Vec3( box.min.x, box.min.y, box.min.z );
	outCorners[1] = Vec3( box.max.x, box.min.y, box.min.z );
	outCorners[2] = Vec3( box.max.x, box.max.y, box.min.z );
	outCorners[3] = Vec3( box.min.x, box.max.y, box.min.z );
	outCorners[4] = Vec3( box.min.x, box.min.y, box.max.z );
	outCorners[5] = Vec3( box.max.x, box.min.y, box.max.z );
	outCorners[6] = Vec3( box.max.x, box.max.y, box.max.z );
	outCorners[7] = Vec3( box.min.x, box.max.y, box.max.z );
}

bool IsSameRotation( const Quat& a, const Quat& b )
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool IsIdentity( const Transform& transform )
{
	return transform.position == Vec3::V_Zero && IsSameRotation( transform.rotation, Quat() ) && transform.scale == Vec3::V_One;
}

// Note: The materials a sub-mesh can use: none, then every material of the model's libraries, each name once
std::vector<std::string> GetMaterialChoices( const ModelData& data )
{
	std::vector<std::string> choices = { "" };
	ResourceManager&         manager = App::GetSession().GetResourceManager();
	for( const std::string& libraryFile : data.materialLibraries )
	{
		const shared<Material> library = manager.FindResource<Material>( std::filesystem::path( libraryFile ).stem().string(), true );
		if( library == nullptr )
			continue;

		for( const MaterialEntry& entry : library->GetEntries() )
		{
			if( std::find( choices.begin(), choices.end(), entry.data.name ) == choices.end() )
			{
				choices.push_back( entry.data.name );
			}
		}
	}
	return choices;
}

// Note: Explains the last item when the mouse rests on it, greyed out items included
void ShowTooltip( const char* text )
{
	ImGui::SetItemTooltip( "%s", text );
}

// Note: A full-width button, so the tools line up in a column, with what it does on hover
bool ToolButton( const char* label, const char* tooltip )
{
	const bool bPressed = ImGui::Button( label, ImVec2( -1.0f, 0.0f ) );
	ShowTooltip( tooltip );
	return bPressed;
}
} // namespace

ModelEditor::ModelEditor( const EditorConfig& config )
    : EditorShell( config )
{
	CreatePreviewScene();
}

ModelEditor::~ModelEditor()
{
	// Note: The preview still points at the model, it draws nothing once released
	if( Entity* modelEntity = FindPreviewEntity( m_ModelId ) )
	{
		if( PreviewModelComponent* preview = modelEntity->GetComponent<PreviewModelComponent>() )
		{
			preview->SetModel( nullptr );
		}
	}
}

bool ModelEditor::OpenModel( std::string_view modelName )
{
	const shared<Model> model = App::GetSession().GetResourceManager().FindResource<Model>( modelName );
	if( model == nullptr )
	{
		GAIA_ERROR( "Model Editor: there is no model called {} in the project", modelName );
		return false;
	}

	return ImportModel( model->GetPath() );
}

bool ModelEditor::ImportModel( std::string_view path )
{
	if( !m_Model.Import( path ) )
		return false;

	m_SelectedMesh    = -1;
	m_SelectedSubMesh = -1;
	SetPendingTransform( Transform() );
	ShowModel();
	FrameModel();
	return true;
}

void ModelEditor::NewModel()
{
	m_Model.Clear();
	m_SelectedMesh    = -1;
	m_SelectedSubMesh = -1;
	SetPendingTransform( Transform() );
	ShowModel();
}

void ModelEditor::FrameModel()
{
	if( Entity* camera = FindPreviewEntity( m_CameraId ) )
	{
		if( OrbitCameraComponent* orbit = camera->GetComponent<OrbitCameraComponent>() )
		{
			orbit->Frame( GetShownBoundingVolume() );
		}
	}
}

bool ModelEditor::ApplyEdit( std::string_view label, const std::function<bool( ModelData& )>& edit )
{
	if( !m_Model.Apply( label, edit ) )
		return false;

	ClampSelection();
	ShowModel();
	return true;
}

bool ModelEditor::Undo()
{
	if( !m_Model.Undo() )
		return false;

	ClampSelection();
	ShowModel();
	return true;
}

bool ModelEditor::Redo()
{
	if( !m_Model.Redo() )
		return false;

	ClampSelection();
	ShowModel();
	return true;
}

bool ModelEditor::ApplyPendingTransform()
{
	const Transform pending = GetPendingTransform();
	if( IsIdentity( pending ) )
		return false;

	const Matrix44 matrix = pending.ToMatrix44();
	SetPendingTransform( Transform() );
	return ApplyEdit( "Apply Transform", [&matrix]( ModelData& data ) {
		ModelOperations::Transform( data, matrix );
		return true;
	} );
}

bool ModelEditor::GetGizmoTarget( Transform& outTransform ) const
{
	if( m_Model.IsEmpty() )
		return false;

	outTransform = GetPendingTransform();
	return true;
}

void ModelEditor::SetGizmoTarget( const Transform& transform )
{
	SetPendingTransform( transform );
}

void ModelEditor::CreatePreviewScene()
{
	Level* level = App::GetSession().GetCurrentLevel();
	if( level == nullptr )
	{
		GAIA_ERROR( "Model Editor: there is no level to preview models in" );
		return;
	}

	Entity* camera = level->CreateEntity( "Camera" );
	camera->AddComponent<OrbitCameraComponent>()->SetHorizontalFOV( 70.0f );
	m_CameraId = camera->GetId();

	Entity* model = level->CreateEntity( "Model" );
	model->AddComponent<PreviewModelComponent>();
	m_ModelId = model->GetId();

	Entity* sun = level->CreateEntity( "Sun" );
	sun->SetRotation( LookAtQuat( kSunDirection, Vec3::V_Up ) );
	shared<DirectionalLightComponent> sunLight = sun->AddComponent<DirectionalLightComponent>();
	sunLight->SetColor( Vec3( 1.0f, 0.95f, 0.85f ) );
	sunLight->SetIntensity( 1.0f );
	m_SunId = sun->GetId();

	Entity* ground = level->CreateEntity( "Ground" );
	ground->AddComponent<ModelComponent>()->SetModel( kGroundModel );
	m_GroundId = ground->GetId();

	PlaceSunAndGround();
}

void ModelEditor::ShowModel()
{
	if( Entity* modelEntity = FindPreviewEntity( m_ModelId ) )
	{
		if( PreviewModelComponent* preview = modelEntity->GetComponent<PreviewModelComponent>() )
		{
			preview->SetModel( m_Model.GetGpuModel() );
		}
	}

	PlaceSunAndGround();
}

void ModelEditor::PlaceSunAndGround()
{
	// Note: Without a model, a unit box at the origin, so the scene still shows a ground
	AABB placed = GetShownBoundingVolume();
	if( placed.IsEmpty() )
	{
		placed.Add( Vec3( -0.5f, 0.0f, -0.5f ) );
		placed.Add( Vec3( 0.5f, 1.0f, 0.5f ) );
	}

	const Vec3  center = placed.GetCenter();
	const float radius = Max( placed.GetRadius(), 0.01f );

	// Note: The shadow box starts at the sun and covers the model's sphere and the ground around it
	if( Entity* sun = FindPreviewEntity( m_SunId ) )
	{
		sun->SetPosition( center - kSunDirection * Scalar( radius * 2.0f ) );
		if( DirectionalLightComponent* sunLight = sun->GetComponent<DirectionalLightComponent>() )
		{
			sunLight->SetShadowSize( radius * 4.0f );
			sunLight->SetShadowDepth( radius * 4.0f );
		}
	}

	if( Entity* ground = FindPreviewEntity( m_GroundId ) )
	{
		ground->SetPosition( Vec3( center.x, placed.min.y, center.z ) );
		ground->SetScale( Vec3( radius * 4.0f, 1.0f, radius * 4.0f ) );
		if( ModelComponent* groundModel = ground->GetComponent<ModelComponent>() )
		{
			groundModel->SetModel( m_bShowGround ? kGroundModel : "" );
		}
	}
}

Entity* ModelEditor::FindPreviewEntity( u64 id ) const
{
	Level* level = App::GetSession().GetCurrentLevel();
	return level != nullptr ? level->FindEntity( id ) : nullptr;
}

Transform ModelEditor::GetPendingTransform() const
{
	const Entity* modelEntity = FindPreviewEntity( m_ModelId );
	return modelEntity != nullptr ? modelEntity->GetTransform() : Transform();
}

void ModelEditor::SetPendingTransform( const Transform& transform )
{
	if( Entity* modelEntity = FindPreviewEntity( m_ModelId ) )
	{
		modelEntity->SetTransform( transform );
	}

	PlaceSunAndGround();
}

AABB ModelEditor::GetShownBoundingVolume() const
{
	const AABB boundingVolume = m_Model.GetData().GetBoundingVolume();
	if( boundingVolume.IsEmpty() )
		return boundingVolume;

	// Note: The bounding volume around the transformed corners, which holds the transformed model
	const Matrix44 matrix = GetPendingTransform().ToMatrix44();
	Vec3           corners[8];
	GetBoxCorners( boundingVolume, corners );

	AABB shown;
	for( const Vec3& corner : corners )
	{
		shown.Add( Vec3( matrix * Vec4( corner, 1.0f ) ) );
	}
	return shown;
}

void ModelEditor::ClampSelection()
{
	const std::vector<MeshData>& meshes = m_Model.GetData().meshes;
	if( m_SelectedMesh >= static_cast<int>( meshes.size() ) )
	{
		m_SelectedMesh    = -1;
		m_SelectedSubMesh = -1;
		return;
	}

	if( m_SelectedMesh >= 0 && m_SelectedSubMesh >= static_cast<int>( meshes[m_SelectedMesh].subMeshes.size() ) )
	{
		m_SelectedSubMesh = -1;
	}
}

void ModelEditor::DrawMenus()
{
	if( ImGui::BeginMenu( "File" ) )
	{
		if( ImGui::MenuItem( "New" ) )
		{
			NewModel();
		}

		const std::vector<std::string> modelNames = App::GetSession().GetResourceManager().GetResourceNames<Model>();
		if( ImGui::BeginMenu( "Open", !modelNames.empty() ) )
		{
			for( const std::string& modelName : modelNames )
			{
				if( ImGui::MenuItem( modelName.c_str() ) )
				{
					OpenModel( modelName );
				}
			}

			ImGui::EndMenu();
		}

		if( ImGui::MenuItem( "Import..." ) )
		{
			m_bImportRequested = true;
		}

		ImGui::EndMenu();
	}

	if( ImGui::BeginMenu( "Edit" ) )
	{
		const std::string undoLabel = m_Model.CanUndo() ? "Undo " + m_Model.GetUndoLabel() : std::string( "Undo" );
		if( ImGui::MenuItem( undoLabel.c_str(), "Ctrl+Z", false, m_Model.CanUndo() ) )
		{
			Undo();
		}

		const std::string redoLabel = m_Model.CanRedo() ? "Redo " + m_Model.GetRedoLabel() : std::string( "Redo" );
		if( ImGui::MenuItem( redoLabel.c_str(), "Ctrl+Y", false, m_Model.CanRedo() ) )
		{
			Redo();
		}

		ImGui::EndMenu();
	}

	if( ImGui::BeginMenu( "View" ) )
	{
		if( ImGui::MenuItem( "Frame Model", "F", false, !m_Model.IsEmpty() ) )
		{
			FrameModel();
		}

		ImGui::Separator();

		if( ImGui::MenuItem( "Ground", nullptr, m_bShowGround ) )
		{
			m_bShowGround = !m_bShowGround;
			PlaceSunAndGround();
		}

		if( ImGui::MenuItem( "Bounding Volume", nullptr, m_bShowBoundingVolume ) )
		{
			m_bShowBoundingVolume = !m_bShowBoundingVolume;
		}

		if( ImGui::MenuItem( "Normals", nullptr, m_bShowNormals ) )
		{
			m_bShowNormals = !m_bShowNormals;
		}
		ShowTooltip( "Draws each vertex normal as a short line, of the selection or of the whole model" );

		ImGui::BeginDisabled( !m_bShowNormals );
		ImGui::SetNextItemWidth( 120.0f * GetDisplayScale() );
		ImGui::SliderFloat( "Normal length", &m_NormalLength, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp );
		ShowTooltip( "How long the normal lines are, as a share of the model's size" );
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
}

void ModelEditor::DrawPanels()
{
	DrawModelPanel();
	DrawInspector();
	DrawStatistics();
	DrawImportPopup();

	// Note: Shortcuts only while no text field is being typed in
	if( ImGui::GetIO().WantTextInput )
		return;

	if( ImGui::IsKeyPressed( ImGuiKey_F, false ) && !m_Model.IsEmpty() )
	{
		FrameModel();
	}

	if( ImGui::IsKeyChordPressed( ImGuiMod_Ctrl | ImGuiKey_Z ) )
	{
		Undo();
	}

	if( ImGui::IsKeyChordPressed( ImGuiMod_Ctrl | ImGuiKey_Y ) || ImGui::IsKeyChordPressed( ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z ) )
	{
		Redo();
	}
}

void ModelEditor::BuildDockLayout( u32& viewportNode )
{
	ImGuiID modelNode     = 0;
	ImGuiID remainingNode = 0;
	ImGui::DockBuilderSplitNode( viewportNode, ImGuiDir_Left, 0.2f, &modelNode, &remainingNode );

	ImGuiID inspectorNode = 0;
	ImGuiID middleNode    = 0;
	ImGui::DockBuilderSplitNode( remainingNode, ImGuiDir_Right, 0.25f, &inspectorNode, &middleNode );

	ImGuiID statisticsNode = 0;
	ImGuiID centerNode     = 0;
	ImGui::DockBuilderSplitNode( middleNode, ImGuiDir_Down, 0.06f, &statisticsNode, &centerNode );

	ImGui::DockBuilderDockWindow( kModelWindow, modelNode );
	ImGui::DockBuilderDockWindow( kInspectorWindow, inspectorNode );
	ImGui::DockBuilderDockWindow( kStatisticsWindow, statisticsNode );
	viewportNode = centerNode;
}

void ModelEditor::DrawViewportOverlay( Vec2 position, Vec2 size )
{
	const ModelData& data   = m_Model.GetData();
	const Matrix44   matrix = GetPendingTransform().ToMatrix44();

	if( m_bShowBoundingVolume )
	{
		DrawBox( data.GetBoundingVolume(), matrix, position, size, ImGui::GetColorU32( kColorTextDisabled ) );
	}

	DrawBox( GetSelectionBoundingVolume( data, m_SelectedMesh, m_SelectedSubMesh ), matrix, position, size, ImGui::GetColorU32( kColorAccentBright ) );

	if( m_bShowNormals )
	{
		DrawNormals( matrix, position, size );
	}
}

void ModelEditor::DrawNormals( const Matrix44& transform, Vec2 position, Vec2 size ) const
{
	const ModelData& data = m_Model.GetData();
	if( data.normals.size() != data.positions.size() )
		return;

	// Note: Normals turn with the inverse transpose, so they stay at right angles to the surface under uneven scale. A
	// flattening transform has no inverse, nothing is drawn then
	const Matrix33 linear( Vec3( transform.col[0] ), Vec3( transform.col[1] ), Vec3( transform.col[2] ) );
	if( fabsf( linear.Determinant() ) < 1e-12f )
		return;

	// Note: The selected sub-mesh's index range, the selected mesh's, or all of them
	std::vector<const SubMeshData*> shownSubMeshes;
	for( int meshIndex = 0; meshIndex < static_cast<int>( data.meshes.size() ); ++meshIndex )
	{
		const std::vector<SubMeshData>& subMeshes = data.meshes[meshIndex].subMeshes;
		for( int subMeshIndex = 0; subMeshIndex < static_cast<int>( subMeshes.size() ); ++subMeshIndex )
		{
			const bool bMeshShown    = m_SelectedMesh < 0 || m_SelectedMesh == meshIndex;
			const bool bSubMeshShown = m_SelectedSubMesh < 0 || m_SelectedSubMesh == subMeshIndex;
			if( bMeshShown && bSubMeshShown )
			{
				shownSubMeshes.push_back( &subMeshes[subMeshIndex] );
			}
		}
	}

	// Note: The vertices those triangles use, each once even when sub-meshes share it
	std::vector<bool> bVertexShown( data.positions.size(), false );
	for( const SubMeshData* subMesh : shownSubMeshes )
	{
		const u64 end = Min<u64>( u64( subMesh->indexOffset ) + subMesh->indexCount, data.indices.size() );
		for( u64 i = subMesh->indexOffset; i < end; ++i )
		{
			if( data.indices[i] < bVertexShown.size() )
			{
				bVertexShown[data.indices[i]] = true;
			}
		}
	}

	const Matrix33 inverse = linear.Inverse();
	const Matrix33 normalMatrix( inverse.GetRow( 0 ), inverse.GetRow( 1 ), inverse.GetRow( 2 ) );

	// Note: The length is a share of the model's size, so it reads the same on a cup and on a building
	const float length = Max( GetShownBoundingVolume().GetRadius(), 0.001f ) * 2.0f * m_NormalLength;

	const Entity* camera         = FindPreviewEntity( m_CameraId );
	const Vec3    cameraPosition = camera != nullptr ? camera->GetPosition() : Vec3::V_Zero;

	ImDrawList* drawList  = ImGui::GetWindowDrawList();
	const float thickness = GetDisplayScale();
	drawList->PushClipRect( ImVec2( position.x, position.y ), ImVec2( position.x + size.x, position.y + size.y ), true );

	for( u64 i = 0; i < data.positions.size(); ++i )
	{
		if( !bVertexShown[i] )
			continue;

		const Vec3 worldPosition = Vec3( transform * Vec4( data.positions[i], 1.0f ) );
		const Vec3 worldNormal   = ( normalMatrix * data.normals[i] ).Normalized();

		// Note: Normals facing away from the camera are left out, the lines aren't hidden behind the model otherwise
		if( Dot( worldNormal, cameraPosition - worldPosition ) < 0.0f )
			continue;

		Vec2 from;
		Vec2 to;
		if( ProjectToViewport( worldPosition, position, size, from ) && ProjectToViewport( worldPosition + worldNormal * Scalar( length ), position, size, to ) )
		{
			// Note: Coloured like the Normals draw mode, each axis as red, green and blue
			const ImU32 color = ImGui::ColorConvertFloat4ToU32( ImVec4( worldNormal.x * 0.5f + 0.5f, worldNormal.y * 0.5f + 0.5f, worldNormal.z * 0.5f + 0.5f, 1.0f ) );
			drawList->AddLine( ImVec2( from.x, from.y ), ImVec2( to.x, to.y ), color, thickness );
		}
	}

	drawList->PopClipRect();
}

void ModelEditor::DrawModelPanel()
{
	if( ImGui::Begin( kModelWindow ) )
	{
		if( m_Model.IsEmpty() )
		{
			ImGui::TextDisabled( "No model, use File > Open or Import" );
		}
		else
		{
			ImGui::TextUnformatted( m_Model.GetName().c_str() );
			ImGui::TextDisabled( "%s", m_Model.GetSourcePath().c_str() );
			ImGui::Separator();

			const std::vector<MeshData>& meshes = m_Model.GetData().meshes;
			for( int meshIndex = 0; meshIndex < static_cast<int>( meshes.size() ); ++meshIndex )
			{
				const MeshData& mesh = meshes[meshIndex];

				ImGuiTreeNodeFlags meshFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				if( meshIndex == m_SelectedMesh && m_SelectedSubMesh < 0 )
				{
					meshFlags |= ImGuiTreeNodeFlags_Selected;
				}

				const std::string meshLabel = ( mesh.name.empty() ? std::string( "(unnamed)" ) : mesh.name ) + "  " + std::to_string( GetMeshTriangleCount( mesh ) ) + " tris";
				const bool        bOpen     = ImGui::TreeNodeEx( reinterpret_cast<void*>( static_cast<intptr_t>( meshIndex ) ), meshFlags, "%s", meshLabel.c_str() );
				if( ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() )
				{
					m_SelectedMesh    = meshIndex;
					m_SelectedSubMesh = -1;
				}

				if( bOpen )
				{
					for( int subMeshIndex = 0; subMeshIndex < static_cast<int>( mesh.subMeshes.size() ); ++subMeshIndex )
					{
						const SubMeshData& subMesh       = mesh.subMeshes[subMeshIndex];
						const std::string  subMeshLabel  = std::string( GetSlotLabel( subMesh.materialSlot ) ) + "  " + std::to_string( subMesh.indexCount / 3 ) + " tris";
						const bool         bSelected     = meshIndex == m_SelectedMesh && subMeshIndex == m_SelectedSubMesh;
						const int          subMeshItemId = meshIndex * 1000 + subMeshIndex;

						ImGui::PushID( subMeshItemId );
						if( ImGui::Selectable( subMeshLabel.c_str(), bSelected ) )
						{
							m_SelectedMesh    = meshIndex;
							m_SelectedSubMesh = subMeshIndex;
						}
						ImGui::PopID();
					}

					ImGui::TreePop();
				}
			}

			// Note: Clicking empty space clears the selection
			if( ImGui::IsWindowHovered() && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && !ImGui::IsAnyItemHovered() )
			{
				m_SelectedMesh    = -1;
				m_SelectedSubMesh = -1;
			}
		}
	}
	ImGui::End();
}

void ModelEditor::DrawInspector()
{
	if( ImGui::Begin( kInspectorWindow ) )
	{
		if( m_Model.IsEmpty() )
		{
			ImGui::TextDisabled( "No model" );
		}
		else
		{
			// Note: The selection's tools first, the whole model's below
			if( m_SelectedMesh >= 0 && m_SelectedSubMesh >= 0 )
			{
				DrawSubMeshTools();
			}
			else if( m_SelectedMesh >= 0 )
			{
				DrawMeshTools();
			}

			DrawTransformTools();
			DrawGeometryTools();
		}
	}
	ImGui::End();
}

void ModelEditor::DrawTransformTools()
{
	PushGreyHeaders();
	const bool bOpen = ImGui::CollapsingHeader( "Transform", ImGuiTreeNodeFlags_DefaultOpen );
	PopGreyHeaders();
	if( !bOpen )
		return;

	// Note: The pending transform, the gizmo moves it too, nothing changes in the model until Apply
	Transform transform = GetPendingTransform();
	if( !IsSameRotation( transform.rotation, m_EulerSource ) )
	{
		m_EulerSource = transform.rotation;
		m_EulerAngles = EulerFromQuat( transform.rotation );
	}

	bool bChanged = Inspector::EditVec3( "Position", transform.position );
	ShowTooltip( "Moves the model, only on screen until Apply" );
	if( Inspector::EditVec3( "Rotation", m_EulerAngles, 0.5f ) )
	{
		transform.rotation = QuatFromEuler( m_EulerAngles );
		m_EulerSource      = transform.rotation;
		bChanged           = true;
	}
	ShowTooltip( "Turns the model, in degrees around X, Y and Z, only on screen until Apply" );
	bChanged |= Inspector::EditVec3( "Scale", transform.scale, 0.01f );
	ShowTooltip( "Scales the model along X, Y and Z, only on screen until Apply" );

	if( bChanged )
	{
		SetPendingTransform( transform );
	}

	const bool bPending = !IsIdentity( transform );
	ImGui::BeginDisabled( !bPending );
	if( ImGui::Button( "Apply" ) )
	{
		ApplyPendingTransform();
	}
	ShowTooltip( "Bakes the position, rotation and scale into the vertices, then sets them back to none" );
	ImGui::SameLine();
	if( ImGui::Button( "Reset" ) )
	{
		SetPendingTransform( Transform() );
	}
	ShowTooltip( "Drops the position, rotation and scale without changing the vertices" );
	ImGui::EndDisabled();

	// Note: Presets change the vertices straight away, each as its own undo step
	auto bake = [this]( std::string_view label, const Transform& preset ) {
		const Matrix44 matrix = preset.ToMatrix44();
		ApplyEdit( label, [&matrix]( ModelData& data ) {
			ModelOperations::Transform( data, matrix );
			return true;
		} );
	};

	ImGui::SeparatorText( "Pivot" );
	const AABB  boundingVolume = m_Model.GetData().GetBoundingVolume();
	const Vec3  center         = boundingVolume.GetCenter();
	const float buttonWidth    = ( ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x ) * 0.5f;

	// Note: Two half-width buttons side by side
	if( ImGui::Button( "To centre", ImVec2( buttonWidth, 0.0f ) ) )
	{
		bake( "Pivot to Centre", Transform( -center, Quat(), Vec3::V_One ) );
	}
	ShowTooltip( "Moves the model so the centre of its bounding volume sits at the origin" );
	ImGui::SameLine();
	if( ImGui::Button( "To base", ImVec2( buttonWidth, 0.0f ) ) )
	{
		bake( "Pivot to Base", Transform( Vec3( -center.x, -boundingVolume.min.y, -center.z ), Quat(), Vec3::V_One ) );
	}
	ShowTooltip( "Moves the model so it stands on the origin, centred in X and Z with its lowest point at Y 0" );
}

void ModelEditor::DrawGeometryTools()
{
	PushGreyHeaders();
	const bool bOpen = ImGui::CollapsingHeader( "Geometry", ImGuiTreeNodeFlags_DefaultOpen );
	PopGreyHeaders();
	if( !bOpen )
		return;

	Inspector::EditSlider( "Smoothing angle", m_SmoothingAngle, 0.0f, 180.0f, "%.0f deg" );
	ShowTooltip( "Faces meeting at up to this angle look smooth, sharper edges stay hard. 0 gives flat faces, 180 smooths everything" );
	if( ToolButton( "Recompute normals", "Works out the normals again from the faces, using the smoothing angle above" ) )
	{
		const float angle = m_SmoothingAngle;
		ApplyEdit( "Recompute Normals", [angle]( ModelData& data ) {
			ModelOperations::ComputeNormals( data, angle );
			return true;
		} );
	}

	if( ToolButton( "Flip faces", "Turns every face around, for models that show their inside" ) )
	{
		ApplyEdit( "Flip Faces", []( ModelData& data ) {
			ModelOperations::FlipFaces( data );
			return true;
		} );
	}

	ImGui::Separator();
	Inspector::EditSlider( "Weld tolerance", m_WeldTolerance, 0.0f, 0.01f, "%.5f" );
	ShowTooltip( "How far apart vertices can be and still be welded, in model units" );

	// Note: Only kept as an undo step when something changed
	if( ToolButton( "Weld vertices", "Merges vertices whose position, normal and UV match within the tolerance above" ) )
	{
		const float tolerance = m_WeldTolerance;
		ApplyEdit( "Weld Vertices", [tolerance]( ModelData& data ) {
			const u64 before = data.positions.size();
			ModelOperations::WeldVertices( data, tolerance );
			return data.positions.size() != before;
		} );
	}

	if( ToolButton( "Remove unused vertices", "Drops the vertices no triangle uses" ) )
	{
		ApplyEdit( "Remove Unused Vertices", []( ModelData& data ) {
			const u64 before = data.positions.size();
			ModelOperations::RemoveUnusedVertices( data );
			return data.positions.size() != before;
		} );
	}
}

void ModelEditor::DrawMeshTools()
{
	PushGreyHeaders();
	const bool bOpen = ImGui::CollapsingHeader( "Mesh", ImGuiTreeNodeFlags_DefaultOpen );
	PopGreyHeaders();
	if( !bOpen )
		return;

	const std::vector<MeshData>& meshes    = m_Model.GetData().meshes;
	const int                    meshIndex = m_SelectedMesh;

	// Note: The name field starts from the mesh's name whenever another mesh is selected
	if( m_MeshNameIndex != meshIndex )
	{
		m_MeshNameIndex = meshIndex;
		std::strncpy( m_MeshName, meshes[meshIndex].name.c_str(), sizeof( m_MeshName ) - 1 );
		m_MeshName[sizeof( m_MeshName ) - 1] = '\0';
	}

	ImGui::InputText( "Name", m_MeshName, sizeof( m_MeshName ) );
	ShowTooltip( "Renames the mesh once you press Enter or click away" );
	if( ImGui::IsItemDeactivatedAfterEdit() )
	{
		const std::string name = m_MeshName;
		ApplyEdit( "Rename Mesh", [meshIndex, name]( ModelData& data ) {
			return ModelOperations::RenameMesh( data, meshIndex, name );
		} );
	}

	const bool bMergeOpen = ImGui::BeginCombo( "Merge into", "Choose a mesh" );
	ShowTooltip( "Moves this mesh's triangles into the chosen mesh, joining sub-meshes that use the same material" );
	if( bMergeOpen )
	{
		for( int target = 0; target < static_cast<int>( meshes.size() ); ++target )
		{
			if( target == meshIndex )
				continue;

			ImGui::PushID( target );
			if( ImGui::Selectable( meshes[target].name.c_str() ) )
			{
				// Note: The target moves up one when the merged mesh came before it
				const int selectAfter = target > meshIndex ? target - 1 : target;
				if( ApplyEdit( "Merge Meshes", [target, meshIndex]( ModelData& data ) { return ModelOperations::MergeMeshes( data, target, meshIndex ); } ) )
				{
					m_SelectedMesh    = selectAfter;
					m_SelectedSubMesh = -1;
					m_MeshNameIndex   = -1;
				}
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	if( ToolButton( "Delete mesh", "Removes the mesh's triangles, and the vertices only they used" ) )
	{
		if( ApplyEdit( "Delete Mesh", [meshIndex]( ModelData& data ) { return ModelOperations::DeleteMesh( data, meshIndex ); } ) )
		{
			m_SelectedMesh    = -1;
			m_SelectedSubMesh = -1;
			m_MeshNameIndex   = -1;
		}
	}
}

void ModelEditor::DrawSubMeshTools()
{
	PushGreyHeaders();
	const bool bOpen = ImGui::CollapsingHeader( "Sub-mesh", ImGuiTreeNodeFlags_DefaultOpen );
	PopGreyHeaders();
	if( !bOpen )
		return;

	const ModelData&   data         = m_Model.GetData();
	const int          meshIndex    = m_SelectedMesh;
	const int          subMeshIndex = m_SelectedSubMesh;
	const SubMeshData& subMesh      = data.meshes[meshIndex].subMeshes[subMeshIndex];

	const bool bMaterialOpen = ImGui::BeginCombo( "Material", GetSlotLabel( subMesh.materialSlot ) );
	ShowTooltip( "Draws the sub-mesh with another material of the model's .mtl libraries. It joins a sub-mesh of the same mesh that already uses it" );
	if( bMaterialOpen )
	{
		for( const std::string& choice : GetMaterialChoices( data ) )
		{
			if( ImGui::Selectable( GetSlotLabel( choice ), choice == subMesh.materialSlot ) && choice != subMesh.materialSlot )
			{
				// Note: The sub-mesh can join another one using that material, so the mesh stays selected instead
				if( ApplyEdit( "Change Material", [meshIndex, subMeshIndex, choice]( ModelData& data ) { return ModelOperations::SetMaterialSlot( data, meshIndex, subMeshIndex, choice ); } ) )
				{
					m_SelectedSubMesh = -1;
				}
				break;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::BeginDisabled( data.meshes[meshIndex].subMeshes.size() < 2 );
	if( ToolButton( "Split into its own mesh", "Moves the sub-mesh into a new mesh placed after this one. Needs at least two sub-meshes" ) )
	{
		if( ApplyEdit( "Split Sub-mesh", [meshIndex, subMeshIndex]( ModelData& data ) { return ModelOperations::SplitSubMesh( data, meshIndex, subMeshIndex ); } ) )
		{
			m_SelectedMesh    = meshIndex + 1;
			m_SelectedSubMesh = -1;
			m_MeshNameIndex   = -1;
		}
	}
	ImGui::EndDisabled();
}

void ModelEditor::DrawStatistics()
{
	if( ImGui::Begin( kStatisticsWindow ) )
	{
		const ModelData& data = m_Model.GetData();

		u64                   subMeshCount = 0;
		std::set<std::string> materialSlots;
		for( const MeshData& mesh : data.meshes )
		{
			subMeshCount += mesh.subMeshes.size();
			for( const SubMeshData& subMesh : mesh.subMeshes )
			{
				materialSlots.insert( subMesh.materialSlot );
			}
		}

		const Vec3 size = data.GetBoundingVolume().GetSize();
		ImGui::Text( "%llu vertices   %llu triangles   %llu meshes   %llu sub-meshes   %llu materials   size %.3f x %.3f x %.3f",
		             static_cast<unsigned long long>( data.positions.size() ),
		             static_cast<unsigned long long>( data.GetTriangleCount() ),
		             static_cast<unsigned long long>( data.meshes.size() ),
		             static_cast<unsigned long long>( subMeshCount ),
		             static_cast<unsigned long long>( materialSlots.size() ),
		             size.x, size.y, size.z );
	}
	ImGui::End();
}

void ModelEditor::DrawImportPopup()
{
	if( m_bImportRequested )
	{
		m_bImportRequested = false;
		ImGui::OpenPopup( kImportPopup );
	}

	if( ImGui::BeginPopupModal( kImportPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::IsWindowAppearing() )
		{
			ImGui::SetKeyboardFocusHere();
		}

		ImGui::TextDisabled( "Full path of an .obj file" );
		ImGui::SetNextItemWidth( 600.0f * GetDisplayScale() );
		const bool bConfirmed = ImGui::InputText( "##Path", m_ImportPath, sizeof( m_ImportPath ), ImGuiInputTextFlags_EnterReturnsTrue );

		ImGui::BeginDisabled( m_ImportPath[0] == '\0' );
		const bool bImportPressed = ImGui::Button( "Import" );
		ImGui::EndDisabled();

		ImGui::SameLine();
		if( ImGui::Button( "Cancel" ) )
		{
			ImGui::CloseCurrentPopup();
		}

		if( ( bConfirmed || bImportPressed ) && m_ImportPath[0] != '\0' )
		{
			ImportModel( m_ImportPath );
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ModelEditor::DrawBox( const AABB& box, const Matrix44& transform, Vec2 position, Vec2 size, u32 color ) const
{
	if( box.IsEmpty() )
		return;

	Vec3 corners[8];
	GetBoxCorners( box, corners );
	for( Vec3& corner : corners )
	{
		corner = Vec3( transform * Vec4( corner, 1.0f ) );
	}

	// Note: The four edges of the near face, the far face, and the four joining them
	constexpr int kEdges[12][2] = {
	    { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	};

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect( ImVec2( position.x, position.y ), ImVec2( position.x + size.x, position.y + size.y ), true );

	for( const auto& edge : kEdges )
	{
		Vec2 from;
		Vec2 to;

		// Note: Edges with a corner behind the camera are left out rather than drawn the wrong way
		if( ProjectToViewport( corners[edge[0]], position, size, from ) && ProjectToViewport( corners[edge[1]], position, size, to ) )
		{
			drawList->AddLine( ImVec2( from.x, from.y ), ImVec2( to.x, to.y ), color, 1.5f * GetDisplayScale() );
		}
	}

	drawList->PopClipRect();
}
