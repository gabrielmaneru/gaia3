#include "gaia3_pch.h"
#include "modelComponent.h"

#include <core/app.h>
#include <gfx/material.h>
#include <gfx/model.h>
#include <scene/entity.h>
#include <scene/inspector.h>

RTTI_DEFINE_COMP( ModelComponent,
                  RTTI_PROPERTY( m_Model ),
                  RTTI_PROPERTY( m_MaterialOverride ) )

void ModelComponent::OnAddComponent()
{
	Renderable::Register();

	ResolveResources();
}

void ModelComponent::OnRemoveComponent()
{
	Renderable::Unregister();

	m_Model.Release();
	m_MaterialOverride.Release();
	m_SubMeshOverrides.clear();
}

void ModelComponent::OnPropertyChanged( std::string_view propertyName )
{
	ResolveResources();
}

void ModelComponent::DrawInspector()
{
	std::string modelName = m_Model.GetName();
	if( Inspector::EditResource( "Model", GetTypeId<Model>(), modelName ) )
	{
		SetModel( modelName );
	}

	const shared<Model>& model           = m_Model.Get();
	auto                 isModelsLibrary = [&model]( std::string_view libraryName ) {
		return model != nullptr && model->UsesMaterialLibrary( libraryName );
	};

	std::string overrideName = m_MaterialOverride.GetName();
	if( Inspector::EditResource( "MaterialOverride", GetTypeId<Material>(), overrideName, isModelsLibrary ) )
	{
		SetMaterialOverride( overrideName );
	}
}

void ModelComponent::ResolveResources()
{
	ResourceManager& manager = App::GetSession().GetResourceManager();

	if( !m_Model.Resolve( manager ) )
	{
		GAIA_ERROR( "ModelComponent: failed to load {}", m_Model.GetName() );
	}

	m_SubMeshOverrides.clear();

	// Note: No override, every sub-mesh draws with the model's own material
	if( m_MaterialOverride.GetName().empty() )
	{
		m_MaterialOverride.Release();
		return;
	}

	if( m_Model.Get() != nullptr && m_Model.Get()->UsesMaterialLibrary( m_MaterialOverride.GetName() ) )
	{
		GAIA_WARN( "ModelComponent: {} already uses material {}, clearing the override", m_Model.GetName(), m_MaterialOverride.GetName() );
		m_MaterialOverride.SetName( "" );
		return;
	}

	const Material* overrideLibrary = nullptr;
	if( m_MaterialOverride.Resolve( manager ) )
	{
		overrideLibrary = m_MaterialOverride.Get().get();
	}
	else
	{
		// Note: The error library stays loaded in the ResourceManager, so nothing here holds on to it
		overrideLibrary = Material::FindOrError( m_MaterialOverride.GetName(), "ModelComponent" ).get();
	}

	if( overrideLibrary != nullptr && m_Model.Get() != nullptr )
	{
		m_SubMeshOverrides = m_Model.Get()->FindMaterials( *overrideLibrary );
	}
}

Matrix44 ModelComponent::GetModelMatrix() const
{
	return GetOwner()->GetTransform().ToMatrix44();
}

void ModelComponent::SetModel( std::string_view modelName )
{
	if( m_Model.GetName() != modelName )
	{
		m_MaterialOverride.SetName( "" );
	}

	m_Model.SetName( modelName );

	// Note: Before the component is on an entity, OnAddComponent resolves it instead
	if( HasOwner() )
	{
		ResolveResources();
	}
}

bool ModelComponent::SetMaterialOverride( std::string_view libraryName )
{
	m_MaterialOverride.SetName( libraryName );

	// Note: Before the component is on an entity, OnAddComponent resolves it instead
	if( HasOwner() )
	{
		ResolveResources();
	}

	// Note: ResolveResources clears an override the model already uses
	return m_MaterialOverride.GetName() == libraryName;
}

void ModelComponent::Draw( const DrawContext& context ) const
{
	if( m_Model.Get() != nullptr )
	{
		m_Model.Get()->Draw( context, m_SubMeshOverrides );
	}
}
