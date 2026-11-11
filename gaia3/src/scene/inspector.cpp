#include "gaia3_pch.h"
#include "inspector.h"

#include <core/app.h>

#include <glad/glad.h>
#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace
{
constexpr const char* kNoResourceLabel = "None";

u32 GetCallbackTexture( const ImDrawCmd* command )
{
	return static_cast<u32>( reinterpret_cast<intptr_t>( command->UserCallbackData ) );
}

// Note: Comparing depth textures only works with shadow samplers, ImGui's plain one reads garbage from them. A depth
// texture also only fills red, so it is spread to every channel to show grey
void ShowDepthAsGrey( const ImDrawList*, const ImDrawCmd* command )
{
	const GLint grey[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
	glBindTexture( GL_TEXTURE_2D, GetCallbackTexture( command ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
	glTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, grey );
}

void RestoreDepthCompare( const ImDrawList*, const ImDrawCmd* command )
{
	const GLint channels[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
	glBindTexture( GL_TEXTURE_2D, GetCallbackTexture( command ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, channels );
}

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

bool EditResourceValue( const std::string& label, u64 resourceTypeId, Json::Value& value )
{
	std::string resourceName = value.asString();
	if( !Inspector::EditResource( label.c_str(), resourceTypeId, resourceName ) )
		return false;

	value = resourceName;
	return true;
}
} // namespace

bool Inspector::EditResource( const char* label, u64 resourceTypeId, std::string& resourceName, const std::function<bool( std::string_view )>& isDisabled )
{
	const std::string currentName = resourceName;
	const char*       preview     = currentName.empty() ? kNoResourceLabel : currentName.c_str();

	bool bChanged = false;
	if( ImGui::BeginCombo( label, preview ) )
	{
		if( ImGui::Selectable( kNoResourceLabel, currentName.empty() ) && !currentName.empty() )
		{
			resourceName = "";
			bChanged     = true;
		}

		for( const std::string& name : App::GetSession().GetResourceManager().GetResourceNamesByTypeId( resourceTypeId ) )
		{
			const ImGuiSelectableFlags flags = isDisabled != nullptr && isDisabled( name ) ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None;
			if( ImGui::Selectable( name.c_str(), name == currentName, flags ) && name != currentName )
			{
				resourceName = name;
				bChanged     = true;
			}
		}

		ImGui::EndCombo();
	}

	return bChanged;
}

bool Inspector::EditFloat( const char* label, float& value, float speed )
{
	return ImGui::DragFloat( label, &value, speed );
}

bool Inspector::EditSlider( const char* label, float& value, float minValue, float maxValue, const char* format )
{
	return ImGui::SliderFloat( label, &value, minValue, maxValue, format, ImGuiSliderFlags_AlwaysClamp );
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
	const bool        bChanged = property.ResourceTypeId != 0 ? EditResourceValue( label, property.ResourceTypeId, value ) : EditJson( label, value );
	if( !bChanged )
		return false;

	return property.Deserialize( valuePtr, value );
}

void Inspector::ShowNote( const char* text )
{
	ImGui::TextDisabled( "%s", text );
}

void Inspector::ShowDepthTexture( u32 textureId, float maxSize )
{
	if( textureId == 0 )
		return;

	const float size = std::min( ImGui::GetContentRegionAvail().x, maxSize );

	// Note: The callbacks run while ImGui renders, around the one image they surround
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddCallback( ShowDepthAsGrey, reinterpret_cast<void*>( static_cast<intptr_t>( textureId ) ) );

	// Note: The UVs are flipped because OpenGL textures start at the bottom left
	ImGui::Image( static_cast<ImTextureID>( textureId ), ImVec2( size, size ), ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );

	drawList->AddCallback( RestoreDepthCompare, reinterpret_cast<void*>( static_cast<intptr_t>( textureId ) ) );
}
