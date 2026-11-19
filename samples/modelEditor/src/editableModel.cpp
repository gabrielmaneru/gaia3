#include "editableModel.h"

#include <core/log.h>
#include <gfx/model.h>
#include <gfx/objImporter.h>

#include <filesystem>

EditableModel::~EditableModel()
{
	ReleaseGpuModel();
}

bool EditableModel::Import( std::string_view path )
{
	// Note: Only OBJ for now, the plan adds other formats through Assimp later
	const std::string extension = std::filesystem::path( path ).extension().string();
	if( extension != ".obj" && extension != ".OBJ" )
	{
		GAIA_ERROR( "Model Editor: can't import {}, only .obj files are supported for now", path );
		return false;
	}

	ModelData data;
	if( !ObjImporter::Import( path, data ) )
		return false;

	// Note: A new GPU model too, it is named after the file it came from
	ReleaseGpuModel();
	m_Data       = std::move( data );
	m_SourcePath = std::string( path );
	m_Name       = std::filesystem::path( path ).stem().string();
	m_UndoSteps.clear();
	m_RedoSteps.clear();
	return Rebuild();
}

void EditableModel::Clear()
{
	ReleaseGpuModel();
	m_Data = ModelData();
	m_Name.clear();
	m_SourcePath.clear();
	m_UndoSteps.clear();
	m_RedoSteps.clear();
}

bool EditableModel::Apply( std::string_view label, const std::function<bool( ModelData& )>& edit )
{
	ModelData before = m_Data;
	if( !edit( m_Data ) )
		return false;

	// Note: The oldest step goes once the history is full
	if( m_UndoSteps.size() >= kMaxUndoSteps )
	{
		m_UndoSteps.erase( m_UndoSteps.begin() );
	}

	m_UndoSteps.push_back( Step{ std::string( label ), std::move( before ) } );
	m_RedoSteps.clear();
	return Rebuild();
}

const std::string& EditableModel::GetUndoLabel() const
{
	static const std::string kNone;
	return m_UndoSteps.empty() ? kNone : m_UndoSteps.back().label;
}

const std::string& EditableModel::GetRedoLabel() const
{
	static const std::string kNone;
	return m_RedoSteps.empty() ? kNone : m_RedoSteps.back().label;
}

bool EditableModel::Undo()
{
	if( m_UndoSteps.empty() )
		return false;

	// Note: The current data becomes the redo step, under the same label
	Step step = std::move( m_UndoSteps.back() );
	m_UndoSteps.pop_back();
	std::swap( m_Data, step.data );
	m_RedoSteps.push_back( std::move( step ) );
	return Rebuild();
}

bool EditableModel::Redo()
{
	if( m_RedoSteps.empty() )
		return false;

	Step step = std::move( m_RedoSteps.back() );
	m_RedoSteps.pop_back();
	std::swap( m_Data, step.data );
	m_UndoSteps.push_back( std::move( step ) );
	return Rebuild();
}

bool EditableModel::Rebuild()
{
	if( m_GpuModel == nullptr )
	{
		// Note: Made from data rather than loaded, so it isn't one of the ResourceManager's models
		m_GpuModel = new_shared<Model>( m_SourcePath, ResourceState::New );
	}

	return m_GpuModel->Build( m_Data );
}

void EditableModel::ReleaseGpuModel()
{
	// Note: Built by hand, so its GPU buffers are freed by hand too
	if( m_GpuModel != nullptr )
	{
		m_GpuModel->UnloadInternal();
		m_GpuModel.reset();
	}
}
