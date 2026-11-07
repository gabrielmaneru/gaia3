#include "gaia3_pch.h"
#include "modelComponent.h"

#include <core/app.h>
#include <gfx/model.h>
#include <scene/entity.h>

RTTI_DEFINE_COMP( ModelComponent,
                  RTTI_PROPERTY( m_Model ) )

ModelComponent::ModelComponent( std::string_view name )
    : m_Model( name )
{
}

void ModelComponent::OnAddComponent()
{
	Renderable::Register();

	ResolveModel();
}

void ModelComponent::OnRemoveComponent()
{
	Renderable::Unregister();

	m_Model.Release();
}

void ModelComponent::OnPropertyChanged( std::string_view propertyName )
{
	ResolveModel();
}

void ModelComponent::ResolveModel()
{
	if( !m_Model.Resolve( App::GetSession().GetResourceManager() ) )
	{
		GAIA_ERROR( "ModelComponent: failed to load {}", m_Model.GetName() );
	}
}

Matrix44 ModelComponent::GetModelMatrix() const
{
	return GetOwner()->GetTransform().ToMatrix44();
}

void ModelComponent::Draw() const
{
	if( m_Model.Get() != nullptr )
	{
		m_Model.Get()->Draw();
	}
}
