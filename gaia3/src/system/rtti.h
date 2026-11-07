#pragma once

#include <core/serializationTypes.h>
#include <core/types.h>
#include <system/type_id.h>
#include <json/json.h>

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace rtti
{

struct PropertyInfo
{
	using SerializeFunc   = bool ( * )( const void* valuePtr, Json::Value& outValue );
	using DeserializeFunc = bool ( * )( void* valuePtr, const Json::Value& inValue );

	const char*     Name;
	u64             TypeId;
	size_t          Offset;
	SerializeFunc   Serialize;
	DeserializeFunc Deserialize;

	// Note: The resource type a ResourceRef property points to, zero for every other property
	u64 ResourceTypeId;
};

struct TypeInfo
{
	const char*         Name;
	u64                 TypeId;
	const PropertyInfo* Properties;
	size_t              PropertyCount;

	const PropertyInfo* FindProperty( std::string_view propertyName ) const;
};

class Registry
{
public:
	static Registry& Instance();

	void            Register( const TypeInfo& typeInfo );
	const TypeInfo* FindByName( std::string_view typeName ) const;
	const TypeInfo* FindById( u64 typeId ) const;

private:
	std::vector<const TypeInfo*> m_Types;
};

template <typename T, typename = void>
struct HasStaticTypeInfo : std::false_type
{
};

template <typename T>
struct HasStaticTypeInfo<T, std::void_t<decltype( T::StaticTypeInfo() )>> : std::true_type
{
};

template <typename T>
struct IsVectorOfUnique : std::false_type
{
};

template <typename T>
struct IsVectorOfUnique<std::vector<unique<T>>> : std::true_type
{
	using ElementType = T;
};

template <typename T>
concept IsResourceRef = requires( T& value, const T& constValue ) {
	typename T::ResourceType;
	constValue.GetName();
	value.SetName( std::string_view{} );
};

template <typename T>
bool ValueToJson( const T& value, Json::Value& outValue )
{
	using ValueType = std::remove_cvref_t<T>;

	if constexpr( IsResourceRef<ValueType> )
	{
		outValue = Json::Value( value.GetName() );
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, bool> )
	{
		outValue = value;
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, std::string> )
	{
		outValue = Json::Value( value );
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, Json::Value> )
	{
		outValue = value;
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, const char*> )
	{
		if( value == nullptr )
		{
			outValue = Json::Value( "" );
			return true;
		}

		outValue = Json::Value( value );
		return true;
	}
	else if constexpr( std::is_enum_v<ValueType> )
	{
		using UnderlyingType = std::underlying_type_t<ValueType>;
		return ValueToJson( static_cast<UnderlyingType>( value ), outValue );
	}
	else if constexpr( std::is_integral_v<ValueType> && std::is_unsigned_v<ValueType> )
	{
		outValue = static_cast<Json::UInt64>( value );
		return true;
	}
	else if constexpr( std::is_integral_v<ValueType> )
	{
		outValue = static_cast<Json::Int64>( value );
		return true;
	}
	else if constexpr( std::is_floating_point_v<ValueType> )
	{
		outValue = static_cast<double>( value );
		return true;
	}
	else if constexpr( HasStaticTypeInfo<ValueType>::value )
	{
		return SerializeObject( &value, ValueType::StaticTypeInfo(), outValue );
	}
	else if constexpr( IsVectorOfUnique<ValueType>::value )
	{
		using ElementType = typename IsVectorOfUnique<ValueType>::ElementType;

		outValue = Json::Value( Json::arrayValue );
		for( const unique<ElementType>& element : value )
		{
			if( element == nullptr )
				continue;

			Json::Value elementJson;
			if( !SerializeObject( element.get(), ElementType::StaticTypeInfo(), elementJson ) )
				return false;

			outValue.append( std::move( elementJson ) );
		}

		return true;
	}

	return false;
}

template <typename T>
bool ValueFromJson( const Json::Value& inValue, T& outValue )
{
	using ValueType = std::remove_cvref_t<T>;

	if constexpr( IsResourceRef<ValueType> )
	{
		if( !inValue.isString() )
			return false;

		outValue.SetName( inValue.asString() );
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, bool> )
	{
		if( !inValue.isBool() )
			return false;

		outValue = inValue.asBool();
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, std::string> )
	{
		if( !inValue.isString() )
			return false;

		outValue = inValue.asString();
		return true;
	}
	else if constexpr( std::is_same_v<ValueType, Json::Value> )
	{
		outValue = inValue;
		return true;
	}
	else if constexpr( std::is_integral_v<ValueType> && std::is_unsigned_v<ValueType> )
	{
		if( !inValue.isUInt64() )
			return false;

		const Json::UInt64 parsedValue = inValue.asUInt64();
		if( parsedValue > std::numeric_limits<ValueType>::max() )
			return false;

		outValue = static_cast<ValueType>( parsedValue );
		return true;
	}
	else if constexpr( std::is_integral_v<ValueType> )
	{
		if( !inValue.isInt64() )
			return false;

		const Json::Int64 parsedValue = inValue.asInt64();
		if( parsedValue < std::numeric_limits<ValueType>::min() || parsedValue > std::numeric_limits<ValueType>::max() )
			return false;

		outValue = static_cast<ValueType>( parsedValue );
		return true;
	}
	else if constexpr( std::is_floating_point_v<ValueType> )
	{
		if( !inValue.isNumeric() )
			return false;

		outValue = static_cast<ValueType>( inValue.asDouble() );
		return true;
	}
	else if constexpr( std::is_enum_v<ValueType> )
	{
		using UnderlyingType = std::underlying_type_t<ValueType>;
		UnderlyingType parsedValue{};
		if( !ValueFromJson( inValue, parsedValue ) )
			return false;

		outValue = static_cast<ValueType>( parsedValue );
		return true;
	}
	else if constexpr( HasStaticTypeInfo<ValueType>::value )
	{
		if( !inValue.isObject() )
			return false;

		return DeserializeObject( &outValue, ValueType::StaticTypeInfo(), inValue );
	}
	else if constexpr( IsVectorOfUnique<ValueType>::value )
	{
		using ElementType = typename IsVectorOfUnique<ValueType>::ElementType;

		if( !inValue.isArray() )
			return false;

		outValue.clear();
		for( const Json::Value& elementJson : inValue )
		{
			unique<ElementType> element = new_unique<ElementType>();
			if( !DeserializeObject( element.get(), ElementType::StaticTypeInfo(), elementJson ) )
				return false;

			outValue.push_back( std::move( element ) );
		}

		return true;
	}

	return false;
}

template <typename T>
PropertyInfo MakeProperty( const char* propertyName, size_t propertyOffset )
{
	u64 resourceTypeId = 0;
	if constexpr( IsResourceRef<T> )
	{
		resourceTypeId = GetTypeId<typename T::ResourceType>();
	}

	return PropertyInfo{ propertyName,
	                     GetTypeId<T>(),
	                     propertyOffset,
	                     []( const void* valuePtr, Json::Value& outValue ) {
		                     return ValueToJson( *reinterpret_cast<const T*>( valuePtr ), outValue );
	                     },
	                     []( void* valuePtr, const Json::Value& inValue ) {
		                     return ValueFromJson( inValue, *reinterpret_cast<T*>( valuePtr ) );
	                     },
	                     resourceTypeId };
}

template <typename T>
TypeInfo CreateTypeInfo( const char* typeName, const PropertyInfo* properties, size_t propertyCount )
{
	return TypeInfo{ typeName, GetTypeId<T>(), properties, propertyCount };
}

bool SerializeObject( const void* instance, const TypeInfo& typeInfo, Json::Value& outObject );
bool DeserializeObject( void* instance, const TypeInfo& typeInfo, const Json::Value& sourceObject );

template <typename T>
bool Serialize( const T& instance, Json::Value& outObject )
{
	return SerializeObject( &instance, T::StaticTypeInfo(), outObject );
}

template <typename T>
bool Deserialize( T& instance, const Json::Value& sourceObject )
{
	return DeserializeObject( &instance, T::StaticTypeInfo(), sourceObject );
}
} // namespace rtti

#define RTTI_DECLARE_TYPE( TypeName )                \
public:                                              \
	static const ::rtti::TypeInfo& StaticTypeInfo(); \
                                                     \
private:

// The property list may be empty: a type can be registered with no properties.
#define RTTI_DEFINE_TYPE( TypeName, ... )                                                              \
	const ::rtti::TypeInfo& TypeName::StaticTypeInfo()                                                 \
	{                                                                                                  \
		using ThisType                                             = TypeName;                         \
		static const std::vector<::rtti::PropertyInfo> kProperties = { __VA_ARGS__ };                  \
		static const ::rtti::TypeInfo                  kTypeInfo   = ::rtti::CreateTypeInfo<TypeName>( \
		    #TypeName, kProperties.data(), kProperties.size() );                                       \
		static const bool kRegistered = []() {                                                         \
			::rtti::Registry::Instance().Register( kTypeInfo );                                        \
			return true;                                                                               \
		}();                                                                                           \
		(void)kRegistered;                                                                             \
		return kTypeInfo;                                                                              \
	}

#define RTTI_PROPERTY( MemberName ) ::rtti::MakeProperty<decltype( ThisType::MemberName )>( #MemberName, offsetof( ThisType, MemberName ) )
