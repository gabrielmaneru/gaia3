#include <gtest/gtest.h>

#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/transform.h>
#include <math/vector.h>
#include <system/rtti.h>

#include <string>
#include <vector>

namespace
{
enum class RttiTestColor : u8
{
	Red   = 0,
	Green = 7
};

class RttiTestChild
{
	RTTI_DECLARE_TYPE( RttiTestChild )
public:
	int value = 0;
};

RTTI_DEFINE_TYPE( RttiTestChild,
                  RTTI_PROPERTY( value ) )

class RttiTestObject
{
	RTTI_DECLARE_TYPE( RttiTestObject )
public:
	bool                               flag     = false;
	int                                count    = 0;
	u64                                big      = 0;
	float                              ratio    = 0.0f;
	double                             precise  = 0.0;
	std::string                        name;
	RttiTestColor                      color = RttiTestColor::Red;
	Transform                          transform;
	RttiTestChild                      child;
	std::vector<unique<RttiTestChild>> children;
};

RTTI_DEFINE_TYPE( RttiTestObject,
                  RTTI_PROPERTY( flag ),
                  RTTI_PROPERTY( count ),
                  RTTI_PROPERTY( big ),
                  RTTI_PROPERTY( ratio ),
                  RTTI_PROPERTY( precise ),
                  RTTI_PROPERTY( name ),
                  RTTI_PROPERTY( color ),
                  RTTI_PROPERTY( transform ),
                  RTTI_PROPERTY( child ),
                  RTTI_PROPERTY( children ) )

class RttiTestEmpty
{
	RTTI_DECLARE_TYPE( RttiTestEmpty )
};

RTTI_DEFINE_TYPE( RttiTestEmpty )

class RttiTestUnsupported
{
	RTTI_DECLARE_TYPE( RttiTestUnsupported )
public:
	std::vector<int> values;
};

RTTI_DEFINE_TYPE( RttiTestUnsupported,
                  RTTI_PROPERTY( values ) )

unique<RttiTestChild> MakeChild( int value )
{
	unique<RttiTestChild> child = new_unique<RttiTestChild>();
	child->value                = value;
	return child;
}

bool DeserializeProperty( const char* propertyName, const Json::Value& value )
{
	Json::Value source( Json::objectValue );
	source[propertyName] = value;

	RttiTestObject object;
	return rtti::Deserialize( object, source );
}
} // namespace

TEST( RttiTests, RoundTripPreservesEveryPropertyKind )
{
	RttiTestObject source;
	source.flag               = true;
	source.count              = -42;
	source.big                = 18446744073709551615ull;
	source.ratio              = 0.1f;
	source.precise            = 1.0 / 3.0;
	source.name               = "gaia3";
	source.color              = RttiTestColor::Green;
	source.transform.position = Vec3( 1.0f, 2.0f, 3.0f );
	source.child.value        = 5;
	source.children.push_back( MakeChild( 8 ) );
	source.children.push_back( MakeChild( 9 ) );

	Json::Value json;
	ASSERT_TRUE( rtti::Serialize( source, json ) );

	RttiTestObject result;
	ASSERT_TRUE( rtti::Deserialize( result, json ) );

	EXPECT_EQ( result.flag, source.flag );
	EXPECT_EQ( result.count, source.count );
	EXPECT_EQ( result.big, source.big );
	EXPECT_EQ( result.ratio, source.ratio );
	EXPECT_EQ( result.precise, source.precise );
	EXPECT_EQ( result.name, source.name );
	EXPECT_EQ( result.color, source.color );
	EXPECT_TRUE( result.transform.position == source.transform.position );
	EXPECT_EQ( result.child.value, source.child.value );
	ASSERT_EQ( result.children.size(), 2u );
	EXPECT_EQ( result.children[0]->value, 8 );
	EXPECT_EQ( result.children[1]->value, 9 );
}

TEST( RttiTests, SerializeUsesPropertyNamesAsKeys )
{
	RttiTestChild child;
	child.value = 3;

	Json::Value json;
	ASSERT_TRUE( rtti::Serialize( child, json ) );

	ASSERT_TRUE( json.isMember( "value" ) );
	EXPECT_EQ( json["value"].asInt(), 3 );
}

TEST( RttiTests, EnumsSerializeAsTheirUnderlyingValue )
{
	RttiTestObject object;
	object.color = RttiTestColor::Green;

	Json::Value json;
	ASSERT_TRUE( rtti::Serialize( object, json ) );

	EXPECT_EQ( json["color"].asInt(), 7 );
}

TEST( RttiTests, DeserializeLeavesMissingPropertiesUntouched )
{
	Json::Value source( Json::objectValue );
	source["count"] = 12;

	RttiTestObject object;
	object.name = "unchanged";
	ASSERT_TRUE( rtti::Deserialize( object, source ) );

	EXPECT_EQ( object.count, 12 );
	EXPECT_EQ( object.name, "unchanged" );
}

TEST( RttiTests, DeserializeIgnoresUnknownProperties )
{
	Json::Value source( Json::objectValue );
	source["doesNotExist"] = 1;

	RttiTestObject object;
	EXPECT_TRUE( rtti::Deserialize( object, source ) );
}

TEST( RttiTests, DeserializeRejectsWrongValueTypes )
{
	EXPECT_FALSE( DeserializeProperty( "flag", 1 ) );
	EXPECT_FALSE( DeserializeProperty( "count", "text" ) );
	EXPECT_FALSE( DeserializeProperty( "big", "text" ) );
	EXPECT_FALSE( DeserializeProperty( "ratio", "text" ) );
	EXPECT_FALSE( DeserializeProperty( "name", 5 ) );
	EXPECT_FALSE( DeserializeProperty( "color", "text" ) );
	EXPECT_FALSE( DeserializeProperty( "transform", 5 ) );
	EXPECT_FALSE( DeserializeProperty( "child", 5 ) );
	EXPECT_FALSE( DeserializeProperty( "children", 5 ) );
}

TEST( RttiTests, DeserializeRejectsNegativeValuesForUnsignedProperties )
{
	EXPECT_FALSE( DeserializeProperty( "big", -1 ) );
	EXPECT_FALSE( DeserializeProperty( "big", -1.0 ) );
}

TEST( RttiTests, DeserializeRejectsFractionsForIntegerProperties )
{
	EXPECT_FALSE( DeserializeProperty( "count", 1.5 ) );
	EXPECT_FALSE( DeserializeProperty( "big", 1.5 ) );
}

TEST( RttiTests, DeserializeRejectsUnsignedValuesTooLargeForSignedProperties )
{
	EXPECT_FALSE( DeserializeProperty( "count", Json::UInt64( 18446744073709551615ull ) ) );
}

TEST( RttiTests, DeserializeRejectsValuesOutsideThePropertyRange )
{
	EXPECT_FALSE( DeserializeProperty( "count", Json::Int64( 5000000000ll ) ) );
	EXPECT_FALSE( DeserializeProperty( "count", Json::Int64( -5000000000ll ) ) );
	EXPECT_FALSE( DeserializeProperty( "color", 300 ) );
}

TEST( RttiTests, DeserializeAcceptsValuesAtThePropertyLimits )
{
	EXPECT_TRUE( DeserializeProperty( "count", 2147483647 ) );
	EXPECT_TRUE( DeserializeProperty( "count", Json::Int64( -2147483648ll ) ) );
	EXPECT_TRUE( DeserializeProperty( "big", Json::UInt64( 18446744073709551615ull ) ) );
	EXPECT_TRUE( DeserializeProperty( "color", 255 ) );
}

TEST( RttiTests, DeserializeAcceptsWholeNumbersStoredAsReals )
{
	EXPECT_TRUE( DeserializeProperty( "count", 3.0 ) );
	EXPECT_TRUE( DeserializeProperty( "big", 3.0 ) );
}

TEST( RttiTests, DeserializeRejectsNonObjectSource )
{
	RttiTestChild child;
	EXPECT_FALSE( rtti::Deserialize( child, Json::Value( 5 ) ) );
	EXPECT_FALSE( rtti::Deserialize( child, Json::Value( Json::arrayValue ) ) );
}

TEST( RttiTests, NullInstancesAreRejected )
{
	const rtti::TypeInfo& typeInfo = RttiTestChild::StaticTypeInfo();

	Json::Value json( Json::objectValue );
	EXPECT_FALSE( rtti::SerializeObject( nullptr, typeInfo, json ) );
	EXPECT_FALSE( rtti::DeserializeObject( nullptr, typeInfo, json ) );
}

TEST( RttiTests, SerializeSkipsNullVectorElements )
{
	RttiTestObject object;
	object.children.push_back( MakeChild( 1 ) );
	object.children.push_back( nullptr );
	object.children.push_back( MakeChild( 2 ) );

	Json::Value json;
	ASSERT_TRUE( rtti::Serialize( object, json ) );

	EXPECT_EQ( json["children"].size(), 2u );
}

TEST( RttiTests, DeserializeReplacesExistingVectorElements )
{
	Json::Value children( Json::arrayValue );
	Json::Value childJson( Json::objectValue );
	childJson["value"] = 4;
	children.append( childJson );

	Json::Value source( Json::objectValue );
	source["children"] = children;

	RttiTestObject object;
	object.children.push_back( MakeChild( 1 ) );
	object.children.push_back( MakeChild( 2 ) );
	ASSERT_TRUE( rtti::Deserialize( object, source ) );

	ASSERT_EQ( object.children.size(), 1u );
	EXPECT_EQ( object.children[0]->value, 4 );
}

TEST( RttiTests, UnsupportedPropertyTypesFailInsteadOfWritingBadData )
{
	RttiTestUnsupported object;

	Json::Value json;
	EXPECT_FALSE( rtti::Serialize( object, json ) );
}

TEST( RttiTests, TypesCanHaveNoProperties )
{
	const rtti::TypeInfo& typeInfo = RttiTestEmpty::StaticTypeInfo();
	EXPECT_EQ( typeInfo.PropertyCount, 0u );
	EXPECT_EQ( typeInfo.FindProperty( "anything" ), nullptr );

	RttiTestEmpty object;
	Json::Value   json;
	ASSERT_TRUE( rtti::Serialize( object, json ) );
	EXPECT_TRUE( json.isObject() );
	EXPECT_TRUE( json.empty() );
	EXPECT_TRUE( rtti::Deserialize( object, json ) );
}

TEST( RttiTests, TypeInfoDescribesItsProperties )
{
	const rtti::TypeInfo& typeInfo = RttiTestChild::StaticTypeInfo();

	EXPECT_STREQ( typeInfo.Name, "RttiTestChild" );
	EXPECT_EQ( typeInfo.TypeId, GetTypeId<RttiTestChild>() );
	ASSERT_EQ( typeInfo.PropertyCount, 1u );

	const rtti::PropertyInfo* property = typeInfo.FindProperty( "value" );
	ASSERT_NE( property, nullptr );
	EXPECT_EQ( property->TypeId, GetTypeId<int>() );
	EXPECT_EQ( typeInfo.FindProperty( "missing" ), nullptr );
}

TEST( RttiTests, RegistryFindsTypesByNameAndId )
{
	const rtti::TypeInfo& typeInfo = RttiTestChild::StaticTypeInfo();
	rtti::Registry&       registry = rtti::Registry::Instance();

	EXPECT_EQ( registry.FindByName( "RttiTestChild" ), &typeInfo );
	EXPECT_EQ( registry.FindById( typeInfo.TypeId ), &typeInfo );
	EXPECT_EQ( registry.FindByName( "NoSuchType" ), nullptr );
	EXPECT_EQ( registry.FindById( 0 ), nullptr );
}

TEST( RttiTests, RegistryIgnoresDuplicateRegistrations )
{
	const rtti::TypeInfo& typeInfo = RttiTestChild::StaticTypeInfo();
	rtti::Registry&       registry = rtti::Registry::Instance();

	const rtti::TypeInfo duplicate = typeInfo;
	registry.Register( duplicate );

	EXPECT_EQ( registry.FindById( typeInfo.TypeId ), &typeInfo );
}
