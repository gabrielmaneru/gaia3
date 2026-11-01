#include "gaia3_pch.h"
#include "rtti.h"

const rtti::PropertyInfo* rtti::TypeInfo::FindProperty( std::string_view propertyName ) const
{
	for( size_t i = 0; i < PropertyCount; ++i )
	{
		if( Properties[i].Name == propertyName )
			return &Properties[i];
	}

	return nullptr;
}

rtti::Registry& rtti::Registry::Instance()
{
	static Registry instance;
	return instance;
}

void rtti::Registry::Register( const TypeInfo& typeInfo )
{
	for( const TypeInfo* existingType : m_Types )
	{
		if( existingType->TypeId == typeInfo.TypeId )
			return;
	}

	m_Types.push_back( &typeInfo );
}

const rtti::TypeInfo* rtti::Registry::FindByName( std::string_view typeName ) const
{
	for( const TypeInfo* typeInfo : m_Types )
	{
		if( typeInfo->Name == typeName )
			return typeInfo;
	}

	return nullptr;
}

const rtti::TypeInfo* rtti::Registry::FindById( u64 typeId ) const
{
	for( const TypeInfo* typeInfo : m_Types )
	{
		if( typeInfo->TypeId == typeId )
			return typeInfo;
	}

	return nullptr;
}

bool rtti::SerializeObject( const void* instance, const TypeInfo& typeInfo, Json::Value& outObject )
{
	if( instance == nullptr )
		return false;

	outObject = Json::Value( Json::objectValue );

	const u8* rawInstance = reinterpret_cast<const u8*>( instance );
	for( size_t i = 0; i < typeInfo.PropertyCount; ++i )
	{
		const PropertyInfo& property      = typeInfo.Properties[i];
		const void*         propertyValue = rawInstance + property.Offset;

		Json::Value serializedValue;
		if( !property.Serialize( propertyValue, serializedValue ) )
			return false;

		outObject[property.Name] = std::move( serializedValue );
	}

	return true;
}

bool rtti::DeserializeObject( void* instance, const TypeInfo& typeInfo, const Json::Value& sourceObject )
{
	if( instance == nullptr || !sourceObject.isObject() )
		return false;

	u8* rawInstance = reinterpret_cast<u8*>( instance );
	for( const std::string& propertyName : sourceObject.getMemberNames() )
	{
		const PropertyInfo* property = typeInfo.FindProperty( propertyName );
		if( property == nullptr )
			continue;

		const Json::Value& serializedValue = sourceObject[propertyName];
		void*              propertyValue   = rawInstance + property->Offset;
		if( !property->Deserialize( propertyValue, serializedValue ) )
			return false;
	}

	return true;
}
