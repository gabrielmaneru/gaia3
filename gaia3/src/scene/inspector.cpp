#include "gaia3_pch.h"
#include "inspector.h"

#include <core/app.h>

#include <imgui.h>

#include <cstring>

namespace
{
constexpr const char* kNoResourceLabel = "None";

// Note: Hides the member prefix, so m_Health is shown as Health
std::string GetDisplayName( std::string_view propertyName )
{
	return std::string( propertyName.starts_with( "m_" ) ? propertyName.substr( 2 ) : propertyName );
}

bool IsVector( const Json::Value& value )
{
	if( !value.isObject() || value.size() < 2 || value.size() > 4 )
		return false;

	for( const std::string& memberName : value.getMemberNames() )
	{
		const bool bIsVectorMember = memberName == "x" || memberName == "y" || memberName == "z" || memberName == "w";
		if( !bIsVectorMember || !value[memberName].isNumeric() )
			return false;
	}

	return true;
}

bool EditVector( const char* label, Json::Value& value )
{
	constexpr const char* kMembers[] = { "x", "y", "z", "w" };

	float      components[4] = {};
	const auto count         = static_cast<int>( value.size() );
	for( int i = 0; i < count; ++i )
	{
		components[i] = value[kMembers[i]].asFloat();
	}

	if( !ImGui::DragScalarN( label, ImGuiDataType_Float, components, count, 0.1f ) )
		return false;

	for( int i = 0; i < count; ++i )
	{
		value[kMembers[i]] = components[i];
	}

	return true;
}

bool EditJson( const std::string& label, Json::Value& value )
{
	if( IsVector( value ) )
	{
		return EditVector( label.c_str(), value );
	}

	if( value.isObject() )
	{
		bool bChanged = false;
		if( ImGui::TreeNodeEx( label.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			for( const std::string& memberName : value.getMemberNames() )
			{
				bChanged |= EditJson( memberName, value[memberName] );
			}

			ImGui::TreePop();
		}

		return bChanged;
	}

	if( value.isArray() )
	{
		bool bChanged = false;
		if( ImGui::TreeNodeEx( label.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			for( Json::ArrayIndex i = 0; i < value.size(); ++i )
			{
				bChanged |= EditJson( std::to_string( i ), value[i] );
			}

			ImGui::TreePop();
		}

		return bChanged;
	}

	switch( value.type() )
	{
	case Json::booleanValue: {
		bool boolValue = value.asBool();
		if( !ImGui::Checkbox( label.c_str(), &boolValue ) )
			return false;

		value = boolValue;
		return true;
	}
	case Json::stringValue: {
		std::string stringValue = value.asString();
		if( !Inspector::EditString( label.c_str(), stringValue ) )
			return false;

		value = stringValue;
		return true;
	}
	case Json::intValue: {
		Json::Int64 intValue = value.asInt64();
		if( !ImGui::DragScalar( label.c_str(), ImGuiDataType_S64, &intValue ) )
			return false;

		value = intValue;
		return true;
	}
	case Json::uintValue: {
		Json::UInt64 uintValue = value.asUInt64();
		if( !ImGui::DragScalar( label.c_str(), ImGuiDataType_U64, &uintValue ) )
			return false;

		value = uintValue;
		return true;
	}
	case Json::realValue: {
		float floatValue = value.asFloat();
		if( !Inspector::EditFloat( label.c_str(), floatValue ) )
			return false;

		value = floatValue;
		return true;
	}
	default:
		ImGui::TextDisabled( "%s: unavailable", label.c_str() );
		return false;
	}
}

bool EditResource( const std::string& label, u64 resourceTypeId, Json::Value& value )
{
	const std::string currentName = value.asString();
	const char*       preview     = currentName.empty() ? kNoResourceLabel : currentName.c_str();

	bool bChanged = false;
	if( ImGui::BeginCombo( label.c_str(), preview ) )
	{
		if( ImGui::Selectable( kNoResourceLabel, currentName.empty() ) && !currentName.empty() )
		{
			value    = "";
			bChanged = true;
		}

		for( const std::string& resourceName : App::GetSession().GetResourceManager().GetResourceNamesByTypeId( resourceTypeId ) )
		{
			if( ImGui::Selectable( resourceName.c_str(), resourceName == currentName ) && resourceName != currentName )
			{
				value    = resourceName;
				bChanged = true;
			}
		}

		ImGui::EndCombo();
	}

	return bChanged;
}
} // namespace

bool Inspector::EditFloat( const char* label, float& value, float speed )
{
	return ImGui::DragFloat( label, &value, speed );
}

bool Inspector::EditVec3( const char* label, Vec3& value, float speed )
{
	return ImGui::DragFloat3( label, &value.x, speed );
}

bool Inspector::EditString( const char* label, std::string& value )
{
	char buffer[256] = {};
	std::strncpy( buffer, value.c_str(), sizeof( buffer ) - 1 );

	if( !ImGui::InputText( label, buffer, sizeof( buffer ) ) )
		return false;

	value = buffer;
	return true;
}

bool Inspector::EditProperty( void* instance, const rtti::PropertyInfo& property )
{
	void* valuePtr = static_cast<u8*>( instance ) + property.Offset;

	Json::Value value;
	if( !property.Serialize( valuePtr, value ) )
	{
		ImGui::TextDisabled( "%s: unavailable", property.Name );
		return false;
	}

	const std::string label    = GetDisplayName( property.Name );
	const bool        bChanged = property.ResourceTypeId != 0 ? EditResource( label, property.ResourceTypeId, value ) : EditJson( label, value );
	if( !bChanged )
		return false;

	return property.Deserialize( valuePtr, value );
}
