#pragma once

#include <gfx/rendererTypes.h>
#include <scene/componentFactory.h>
#include <system/resourceRef.h>

#include <vector>

class Material;
class Model;
struct MaterialEntry;

class ModelComponent : public Component, public Renderable
{
	RTTI_DECLARE_COMP( ModelComponent, Component )
public:
	ModelComponent() = default;

	void OnAddComponent() override;
	void OnRemoveComponent() override;
	void OnPropertyChanged( std::string_view propertyName ) override;

	void DrawInspector() override;

	Matrix44 GetModelMatrix() const override;

	// Note: A different model clears the material override
	void SetModel( std::string_view modelName );

	// Note: An empty name goes back to the model's own material libraries
	bool SetMaterialOverride( std::string_view libraryName );

private:
	void Draw( const DrawContext& context ) const override;
	void DrawShadow( const DrawContext& context, const Shader& shadowShader ) const override;
	void ResolveResources();

	ResourceRef<Model> m_Model;

	// Note: A material library used instead of the model's own, each sub-mesh finds its material name in it. Empty
	// uses the model's own libraries
	ResourceRef<Material> m_MaterialOverride;

	// Note: The override library's material for each sub-mesh, in sub-mesh order, looked up once when the override or
	// the model changes so drawing doesn't search by name. Empty when there is no override
	std::vector<const MaterialEntry*> m_SubMeshOverrides;
};
