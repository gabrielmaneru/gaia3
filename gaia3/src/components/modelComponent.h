#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>
#include <system/resourceRef.h>

class Model;

class ModelComponent : public Component, public Renderable
{
	RTTI_DECLARE_COMP( ModelComponent, Component )
public:
	ModelComponent() = default;
	explicit ModelComponent( std::string_view name );

	void OnAddComponent() override;
	void OnRemoveComponent() override;
	void OnPropertyChanged( std::string_view propertyName ) override;

	Matrix44 GetModelMatrix() const override;

private:
	void Draw() const override;
	void ResolveModel();

	ResourceRef<Model> m_Model;
};
