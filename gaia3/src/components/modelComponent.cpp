#include "gaia3_pch.h"
#include "modelComponent.h"

RTTI_DEFINE_COMP( ModelComponent,
                  RTTI_PROPERTY( modelName ) )

void ModelComponent::OnAddComponent()
{
	Renderable::Register();
}

void ModelComponent::OnRemoveComponent()
{
	Renderable::Unregister();
}

void ModelComponent::Draw() const
{
}
