#pragma once

#include <system/rtti.h>

#include <string_view>

class Entity;

class Component
{
public:
	Component()          = default;
	virtual ~Component() = default;

	virtual void OnAddComponent() {};
	virtual void OnRemoveComponent() {};

	virtual void Update() {};

	// Note: Called after the editor changes a property or the owner's transform
	virtual void OnPropertyChanged( std::string_view propertyName ) {};
	virtual void OnTransformChanged() {};

	// Note: The default edits every RTTI property, override it to build a dedicated editing experience
	virtual void DrawInspector();

	// Forces derived components to use RTTI_DECLARE_COMP
	virtual const rtti::TypeInfo& GetTypeInfo() const = 0;

	Entity* GetOwner() const { return m_Owner; }
	bool    HasOwner() const { return m_Owner != nullptr; }

private:
	friend class Entity;
	void SetOwner( Entity* owner ) { m_Owner = owner; }

	Entity* m_Owner = nullptr;
};