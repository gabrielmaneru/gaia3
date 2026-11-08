#include "gaia3_pch.h"
#include "modelComponent.h"

#include <core/app.h>
#include <gfx/material.h>
#include <gfx/model.h>
#include <scene/entity.h>

RTTI_DEFINE_COMP( ModelComponent,
                  RTTI_PROPERTY( m_Model ),
                  RTTI_PROPERTY( m_Material ) )

ModelComponent::ModelComponent( std::string_view modelName, std::string_view materialName )
    : m_Model( modelName ), m_Material( materialName )
{
}

void ModelComponent::OnAddComponent()
{
	Renderable::Register();

	ResolveResources();
}

void ModelComponent::OnRemoveComponent()
{
	Renderable::Unregister();

	m_Model.Release();
	m_Material.Release();
	m_MissingMaterial.reset();
}

void ModelComponent::OnPropertyChanged( std::string_view propertyName )
{
	ResolveResources();
}

void ModelComponent::ResolveResources()
{
	ResourceManager& manager = App::GetSession().GetResourceManager();

	if( !m_Model.Resolve( manager ) )
	{
		GAIA_ERROR( "ModelComponent: failed to load {}", m_Model.GetName() );
	}

	m_MissingMaterial.reset();
	if( !m_Material.Resolve( manager ) )
	{
		m_MissingMaterial = Material::FindOrError( m_Material.GetName(), "ModelComponent" );
	}
}

Matrix44 ModelComponent::GetModelMatrix() const
{
	return GetOwner()->GetTransform().ToMatrix44();
}

void ModelComponent::Draw( const DrawContext& context ) const
{
	if( m_Model.Get() != nullptr )
	{
		const Material* overrideMaterial = m_Material.Get() != nullptr ? m_Material.Get().get() : m_MissingMaterial.get();
		m_Model.Get()->Draw( context, overrideMaterial );
	}
}
