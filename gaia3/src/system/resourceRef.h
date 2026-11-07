#pragma once

#include <system/resourceManager.h>

#include <string>
#include <string_view>

// Note: Saved as the resource's name, RTTI restores the name and Resolve turns it back into the resource
template <typename TResource>
class ResourceRef
{
public:
	using ResourceType = TResource;

	ResourceRef() = default;
	explicit ResourceRef( std::string_view name )
	    : m_Name( name )
	{
	}

	const std::string&       GetName() const { return m_Name; }
	const shared<TResource>& Get() const { return m_Resource; }

	void SetName( std::string_view name )
	{
		m_Name = name;
		m_Resource.reset();
	}

	// Note: An empty name is a valid choice that means no resource
	bool Resolve( ResourceManager& manager )
	{
		m_Resource = m_Name.empty() ? nullptr : manager.FindResource<TResource>( m_Name, true );
		return m_Name.empty() || m_Resource != nullptr;
	}

	void Release() { m_Resource.reset(); }

private:
	std::string       m_Name;
	shared<TResource> m_Resource;
};
