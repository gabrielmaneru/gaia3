#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>

class ModelComponent : public Component, public Renderable
{
	RTTI_DECLARE_COMP( ModelComponent, Component )
public:
	ModelComponent() = default;

	void OnAddComponent() override;
	void OnRemoveComponent() override;

private:
	void Draw() const override;

	std::string modelName = "TestModel.obj";
};
