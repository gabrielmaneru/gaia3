#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>

class Model;

// Note: Draws a Model it is handed directly, like the one the editor builds from the data it edits. ModelComponent
// finds its models by name, and an edited model isn't a named resource until it is saved
class PreviewModelComponent : public Component, public Renderable
{
	RTTI_DECLARE_COMP( PreviewModelComponent, Component )
public:
	PreviewModelComponent() = default;

	void OnAddComponent() override;
	void OnRemoveComponent() override;

	Matrix44 GetModelMatrix() const override;
	void     Draw( const DrawContext& context ) const override;
	void     DrawShadow( const DrawContext& context, const Shader& shadowShader ) const override;

	// Note: Null draws nothing
	void SetModel( const shared<Model>& model ) { m_Model = model; }

private:
	shared<Model> m_Model;
};
