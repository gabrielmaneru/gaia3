#include "previewModelComponent.h"

#include <gfx/model.h>
#include <scene/entity.h>

RTTI_DEFINE_COMP( PreviewModelComponent )

void PreviewModelComponent::OnAddComponent()
{
	Renderable::Register();
}

void PreviewModelComponent::OnRemoveComponent()
{
	Renderable::Unregister();
	m_Model.reset();
}

Matrix44 PreviewModelComponent::GetModelMatrix() const
{
	return GetOwner()->GetTransform().ToMatrix44();
}

void PreviewModelComponent::Draw( const DrawContext& context ) const
{
	if( m_Model != nullptr )
	{
		m_Model->Draw( context, {} );
	}
}

void PreviewModelComponent::DrawShadow( const DrawContext& context, const Shader& shadowShader ) const
{
	if( m_Model != nullptr )
	{
		m_Model->DrawShadow( context, {}, shadowShader );
	}
}
