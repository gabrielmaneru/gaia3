#include "gaia3_pch.h"
#include "serializationTypes.h"

#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/transform.h>
#include <math/vector.h>
#include <system/rtti.h>

namespace rtti
{

bool ValueToJson( const Vec2& value, Json::Value& outValue )
{
	outValue      = Json::Value( Json::objectValue );
	outValue["x"] = value.x;
	outValue["y"] = value.y;
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Vec2& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "x" ) || !inValue.isMember( "y" ) )
		return false;

	return ValueFromJson( inValue["x"], outValue.x )
	       && ValueFromJson( inValue["y"], outValue.y );
}

bool ValueToJson( const Vec3& value, Json::Value& outValue )
{
	outValue      = Json::Value( Json::objectValue );
	outValue["x"] = value.x;
	outValue["y"] = value.y;
	outValue["z"] = value.z;
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Vec3& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "x" ) || !inValue.isMember( "y" ) || !inValue.isMember( "z" ) )
		return false;

	return ValueFromJson( inValue["x"], outValue.x )
	       && ValueFromJson( inValue["y"], outValue.y )
	       && ValueFromJson( inValue["z"], outValue.z );
}

bool ValueToJson( const Vec4& value, Json::Value& outValue )
{
	outValue      = Json::Value( Json::objectValue );
	outValue["x"] = value.x;
	outValue["y"] = value.y;
	outValue["z"] = value.z;
	outValue["w"] = value.w;
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Vec4& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "x" ) || !inValue.isMember( "y" ) || !inValue.isMember( "z" ) || !inValue.isMember( "w" ) )
		return false;

	return ValueFromJson( inValue["x"], outValue.x )
	       && ValueFromJson( inValue["y"], outValue.y )
	       && ValueFromJson( inValue["z"], outValue.z )
	       && ValueFromJson( inValue["w"], outValue.w );
}

bool ValueToJson( const Quat& value, Json::Value& outValue )
{
	outValue      = Json::Value( Json::objectValue );
	outValue["x"] = value.x;
	outValue["y"] = value.y;
	outValue["z"] = value.z;
	outValue["w"] = value.w;
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Quat& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "x" ) || !inValue.isMember( "y" ) || !inValue.isMember( "z" ) || !inValue.isMember( "w" ) )
		return false;

	return ValueFromJson( inValue["x"], outValue.x )
	       && ValueFromJson( inValue["y"], outValue.y )
	       && ValueFromJson( inValue["z"], outValue.z )
	       && ValueFromJson( inValue["w"], outValue.w );
}

bool ValueToJson( const Matrix33& value, Json::Value& outValue )
{
	outValue = Json::Value( Json::objectValue );

	Json::Value row0;
	Json::Value row1;
	Json::Value row2;
	if( !ValueToJson( value.GetRow( 0 ), row0 ) )
		return false;
	if( !ValueToJson( value.GetRow( 1 ), row1 ) )
		return false;
	if( !ValueToJson( value.GetRow( 2 ), row2 ) )
		return false;

	outValue["row0"] = std::move( row0 );
	outValue["row1"] = std::move( row1 );
	outValue["row2"] = std::move( row2 );
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Matrix33& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "row0" ) || !inValue.isMember( "row1" ) || !inValue.isMember( "row2" ) )
		return false;

	Vec3 row0;
	Vec3 row1;
	Vec3 row2;
	if( !ValueFromJson( inValue["row0"], row0 ) )
		return false;
	if( !ValueFromJson( inValue["row1"], row1 ) )
		return false;
	if( !ValueFromJson( inValue["row2"], row2 ) )
		return false;

	outValue.SetRow( 0, row0 );
	outValue.SetRow( 1, row1 );
	outValue.SetRow( 2, row2 );
	return true;
}

bool ValueToJson( const Matrix44& value, Json::Value& outValue )
{
	outValue = Json::Value( Json::objectValue );

	Json::Value row0;
	Json::Value row1;
	Json::Value row2;
	Json::Value row3;
	if( !ValueToJson( value.GetRow( 0 ), row0 ) )
		return false;
	if( !ValueToJson( value.GetRow( 1 ), row1 ) )
		return false;
	if( !ValueToJson( value.GetRow( 2 ), row2 ) )
		return false;
	if( !ValueToJson( value.GetRow( 3 ), row3 ) )
		return false;

	outValue["row0"] = std::move( row0 );
	outValue["row1"] = std::move( row1 );
	outValue["row2"] = std::move( row2 );
	outValue["row3"] = std::move( row3 );
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Matrix44& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "row0" ) || !inValue.isMember( "row1" ) || !inValue.isMember( "row2" ) || !inValue.isMember( "row3" ) )
		return false;

	Vec4 row0;
	Vec4 row1;
	Vec4 row2;
	Vec4 row3;
	if( !ValueFromJson( inValue["row0"], row0 ) )
		return false;
	if( !ValueFromJson( inValue["row1"], row1 ) )
		return false;
	if( !ValueFromJson( inValue["row2"], row2 ) )
		return false;
	if( !ValueFromJson( inValue["row3"], row3 ) )
		return false;

	outValue.SetRow( 0, row0 );
	outValue.SetRow( 1, row1 );
	outValue.SetRow( 2, row2 );
	outValue.SetRow( 3, row3 );
	return true;
}

bool ValueToJson( const Transform& value, Json::Value& outValue )
{
	outValue = Json::Value( Json::objectValue );

	Json::Value position;
	Json::Value rotation;
	Json::Value scale;
	if( !ValueToJson( value.position, position ) )
		return false;
	if( !ValueToJson( value.rotation, rotation ) )
		return false;
	if( !ValueToJson( value.scale, scale ) )
		return false;

	outValue["position"] = std::move( position );
	outValue["rotation"] = std::move( rotation );
	outValue["scale"]    = std::move( scale );
	return true;
}

bool ValueFromJson( const Json::Value& inValue, Transform& outValue )
{
	if( !inValue.isObject() )
		return false;

	if( !inValue.isMember( "position" ) || !inValue.isMember( "rotation" ) || !inValue.isMember( "scale" ) )
		return false;

	return ValueFromJson( inValue["position"], outValue.position )
	       && ValueFromJson( inValue["rotation"], outValue.rotation )
	       && ValueFromJson( inValue["scale"], outValue.scale );
}

} // namespace rtti
