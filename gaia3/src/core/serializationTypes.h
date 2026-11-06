#pragma once

struct Vec2;
struct Vec3;
struct Vec4;
struct Quat;
struct Matrix33;
struct Matrix44;
struct Transform;
namespace Json
{
class Value;
}

namespace rtti
{

bool ValueToJson( const Vec2& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Vec2& outValue );

bool ValueToJson( const Vec3& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Vec3& outValue );

bool ValueToJson( const Vec4& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Vec4& outValue );

bool ValueToJson( const Quat& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Quat& outValue );

bool ValueToJson( const Matrix33& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Matrix33& outValue );

bool ValueToJson( const Matrix44& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Matrix44& outValue );

bool ValueToJson( const Transform& value, Json::Value& outValue );
bool ValueFromJson( const Json::Value& inValue, Transform& outValue );

} // namespace rtti
