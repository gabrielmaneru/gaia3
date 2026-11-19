#pragma once

#include <gfx/modelData.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

class Model;

// Note: The model being edited: its ModelData, which edits change, and the GPU Model built from it to draw. The GPU
// copy is rebuilt after every change. Each edit keeps a copy of the data from before it, so it can be undone
class EditableModel
{
public:
	// Note: How many edits can be undone. Each keeps a whole copy of the data, about 10 MB for Sponza
	static constexpr u64 kMaxUndoSteps = 16;

	EditableModel() = default;
	~EditableModel();

	EditableModel( const EditableModel& )            = delete;
	EditableModel& operator=( const EditableModel& ) = delete;

	// Note: Reads a model file, keeping the current model when it fails. Starts a new undo history
	bool Import( std::string_view path );

	// Note: Back to no model
	void Clear();

	// Note: Runs edit on the data and rebuilds the GPU model. edit returns false when it changed nothing, then no undo
	// step is kept. label names the step in the Edit menu
	bool Apply( std::string_view label, const std::function<bool( ModelData& )>& edit );

	bool               CanUndo() const { return !m_UndoSteps.empty(); }
	bool               CanRedo() const { return !m_RedoSteps.empty(); }
	const std::string& GetUndoLabel() const;
	const std::string& GetRedoLabel() const;
	bool               Undo();
	bool               Redo();

	bool IsEmpty() const { return m_Data.positions.empty(); }

	const ModelData&     GetData() const { return m_Data; }
	const std::string&   GetName() const { return m_Name; }
	const std::string&   GetSourcePath() const { return m_SourcePath; }
	const shared<Model>& GetGpuModel() const { return m_GpuModel; }

private:
	struct Step
	{
		std::string label;
		ModelData   data;
	};

	// Note: Uploads m_Data again, needs the OpenGL context
	bool Rebuild();
	void ReleaseGpuModel();

	ModelData     m_Data;
	std::string   m_Name;
	std::string   m_SourcePath;
	shared<Model> m_GpuModel;

	std::vector<Step> m_UndoSteps;
	std::vector<Step> m_RedoSteps;
};
