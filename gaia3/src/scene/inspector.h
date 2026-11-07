#pragma once

#include <math/vector.h>
#include <system/rtti.h>

#include <string>

// Note: Widgets for Component::DrawInspector, so components can edit values without depending on ImGui
namespace Inspector
{
bool EditFloat( const char* label, float& value, float speed = 0.1f );
bool EditVec3( const char* label, Vec3& value, float speed = 0.1f );
bool EditString( const char* label, std::string& value );

// Note: Picks the editor from the property's serialized shape, and returns true once the new value is applied
bool EditProperty( void* instance, const rtti::PropertyInfo& property );
} // namespace Inspector
