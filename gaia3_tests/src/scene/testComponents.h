#pragma once

#include <scene/componentFactory.h>

class HealthComponent : public Component
{
	RTTI_DECLARE_COMP( HealthComponent, Component )
public:
	HealthComponent() = default;
	explicit HealthComponent( int health )
	    : m_Health( health )
	{
	}

	void OnAddComponent() override { ++addCount; }
	void OnRemoveComponent() override { ++removeCount; }
	void Update() override { ++updateCount; }

	int m_Health    = 100;
	int addCount    = 0;
	int removeCount = 0;
	int updateCount = 0;
};

class ArmorComponent : public Component
{
	RTTI_DECLARE_COMP( ArmorComponent, Component )
public:
	float m_Armor = 1.5f;
};
