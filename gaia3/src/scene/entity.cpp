#include "gaia3_pch.h"
#include "entity.h"

RTTI_DEFINE_TYPE( Entity,
                  RTTI_PROPERTY( m_Id ),
                  RTTI_PROPERTY( m_Name ),
                  RTTI_PROPERTY( m_Transform ) )

Entity::Entity( u64 id, std::string_view entityName )
    : m_Id( id ), m_Name( entityName ), m_Transform( Transform::V_Identity )
{
}