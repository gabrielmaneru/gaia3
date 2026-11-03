#pragma once

#include <system/rtti.h>

class Entity;

class Component
{
public:
	Component()          = default;
	virtual ~Component() = default;

	virtual void OnAddComponent() {};
	virtual void OnRemoveComponent() {};

	virtual void Update() {};

	// Forces derived components to use RTTI_DECLARE_COMP
	virtual const rtti::TypeInfo& GetTypeInfo() const = 0;

	Entity* GetOwner() const { return m_Owner; }

private:
	friend class Entity;
	void SetOwner( Entity* owner ) { m_Owner = owner; }

	Entity* m_Owner = nullptr;
};