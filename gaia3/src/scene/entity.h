#pragma once

#include <system/rtti.h>

#include <string>
#include <string_view>

class Entity
{
	RTTI_DECLARE_TYPE( Entity )
public:
	Entity() = default;
	Entity( u64 id, std::string_view entityName );

	u64  GetId() const { return m_Id; }
	std::string_view GetName() const { return m_Name; }

private:
	u64                            m_Id   = 0;
	std::string                    m_Name = {};
};