#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>

class Model;

class ModelComponent : public Component, public Renderable
{
	RTTI_DECLARE_COMP( ModelComponent, Component )
public:
	ModelComponent() = default;
	explicit ModelComponent( std::string_view name );

	void OnAddComponent() override;
	void OnRemoveComponent() override;

	Matrix44 GetModelMatrix() const override;

private:
	void Draw() const override;

	std::string   modelName;
	shared<Model> m_Model;
};
