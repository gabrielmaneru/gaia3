#pragma once

#include "editableModel.h"

#include <editor/editorShell.h>
#include <math/boundingBox.h>
#include <math/quaternion.h>
#include <math/transform.h>

#include <functional>
#include <string_view>

class Entity;

// Note: An editor for one model at a time instead of levels. It draws the model in a hidden preview level the user
// never sees: an orbit camera, the model, a sun and a ground plane. Its panels list the model's meshes and sub-meshes,
// the viewport outlines the selected one, and the Inspector edits the model, a mesh or a sub-mesh. The model entity's
// transform is a pending transform that the gizmo and the Inspector move, and Apply bakes into the vertices
class ModelEditor : public EditorShell
{
public:
	explicit ModelEditor( const EditorConfig& config );
	~ModelEditor() override;

	// Note: Opens one of the project's models by name, as File > Open lists them
	bool OpenModel( std::string_view modelName );

	// Note: Opens a model file from anywhere. Its materials are only found when their .mtl is in the project
	bool ImportModel( std::string_view path );

	void NewModel();

	// Note: Points the camera at the whole model
	void FrameModel();

	// Note: Runs an edit as one undo step, then shows the result. Selection that no longer exists is cleared
	bool ApplyEdit( std::string_view label, const std::function<bool( ModelData& )>& edit );
	bool Undo();
	bool Redo();

	// Note: Bakes the pending transform into the vertices and resets it
	bool ApplyPendingTransform();

protected:
	void DrawMenus() override;
	void DrawPanels() override;
	void BuildDockLayout( u32& viewportNode ) override;
	void DrawViewportOverlay( Vec2 position, Vec2 size ) override;
	bool GetGizmoTarget( Transform& outTransform ) const override;
	void SetGizmoTarget( const Transform& transform ) override;

private:
	void    CreatePreviewScene();
	void    ShowModel();
	void    PlaceSunAndGround();
	Entity* FindPreviewEntity( u64 id ) const;

	// Note: The pending transform, identity once applied
	Transform GetPendingTransform() const;
	void      SetPendingTransform( const Transform& transform );

	// Note: The model's bounding volume as it is drawn, after the pending transform
	AABB GetShownBoundingVolume() const;
	void ClampSelection();

	void DrawModelPanel();
	void DrawStatistics();
	void DrawImportPopup();
	void DrawInspector();
	void DrawTransformTools();
	void DrawGeometryTools();
	void DrawMeshTools();
	void DrawSubMeshTools();
	void DrawBox( const AABB& box, const Matrix44& transform, Vec2 position, Vec2 size, u32 color ) const;
	void DrawNormals( const Matrix44& transform, Vec2 position, Vec2 size ) const;

	EditableModel m_Model;

	// Note: The preview level's entities
	u64 m_CameraId = 0;
	u64 m_ModelId  = 0;
	u64 m_SunId    = 0;
	u64 m_GroundId = 0;

	// Note: -1 when nothing is selected, a mesh alone selects all its sub-meshes
	int m_SelectedMesh    = -1;
	int m_SelectedSubMesh = -1;

	bool  m_bShowGround         = true;
	bool  m_bShowBoundingVolume = false;
	bool  m_bShowNormals        = false;
	float m_NormalLength        = 0.02f; // share of the model's size
	bool  m_bImportRequested    = false;
	char  m_ImportPath[512]     = {};
	char  m_MeshName[128]       = {};
	int   m_MeshNameIndex       = -1;
	float m_SmoothingAngle      = 45.0f; // deg
	float m_WeldTolerance       = 0.0001f;

	// Note: Euler angles are kept between frames, reading them back from the rotation makes them jump while dragging
	Quat m_EulerSource;
	Vec3 m_EulerAngles;
};
