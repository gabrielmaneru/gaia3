#include "gaia3_pch.h"
#include "modelComponent.h"

#include <core/app.h>
#include <gfx/model.h>
#include <scene/entity.h>

RTTI_DEFINE_COMP( ModelComponent,
                  RTTI_PROPERTY( modelName ) )

ModelComponent::ModelComponent( std::string_view name )
    : modelName( name )
{
}

void ModelComponent::OnAddComponent()
{
	Renderable::Register();

	m_Model = App::GetSession().GetResourceManager().FindResource<Model>( modelName, true );
	if( m_Model == nullptr )
	{
		std::cerr << "ModelComponent: failed to load " << modelName << '\n';
	}
}

void ModelComponent::OnRemoveComponent()
{
	Renderable::Unregister();

	m_Model.reset();
}

Matrix44 ModelComponent::GetModelMatrix() const
{
	return GetOwner()->GetTransform().ToMatrix44();
}

void ModelComponent::Draw() const
{
	if( m_Model != nullptr )
	{
		m_Model->Draw();
	}
}
