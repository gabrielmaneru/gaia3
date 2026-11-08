#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>
#include <system/resourceRef.h>

class Material;
class Model;

class ModelComponent : public Component, public Renderable
{
	RTTI_DECLARE_COMP( ModelComponent, Component )
public:
	ModelComponent() = default;
	explicit ModelComponent( std::string_view modelName, std::string_view materialName = "" );

	void OnAddComponent() override;
	void OnRemoveComponent() override;
	void OnPropertyChanged( std::string_view propertyName ) override;

	Matrix44 GetModelMatrix() const override;

private:
	void Draw( const DrawContext& context ) const override;
	void ResolveResources();

	ResourceRef<Model> m_Model;

	// Note: Empty uses the model's own materials, otherwise every sub-mesh draws with this one
	ResourceRef<Material> m_Material;

	// Note: The error material, while m_Material names a material that can't be found
	shared<Material> m_MissingMaterial;
};
