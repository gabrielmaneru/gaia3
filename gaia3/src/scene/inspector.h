#pragma once

#include <math/vector.h>
#include <system/rtti.h>

#include <functional>
#include <string>
#include <string_view>

// Note: Widgets for Component::DrawInspector, so components can edit values without depending on ImGui
namespace Inspector
{
bool EditFloat( const char* label, float& value, float speed = 0.1f );
bool EditVec3( const char* label, Vec3& value, float speed = 0.1f );
bool EditString( const char* label, std::string& value );

// Note: A dropdown of every resource of that type plus None, names that isDisabled returns true for are greyed out
bool EditResource( const char* label, u64 resourceTypeId, std::string& resourceName, const std::function<bool( std::string_view )>& isDisabled = nullptr );

// Note: Picks the editor from the property's serialized shape, and returns true once the new value is applied
bool EditProperty( void* instance, const rtti::PropertyInfo& property );
} // namespace Inspector
