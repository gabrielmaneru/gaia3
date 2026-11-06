#include <gtest/gtest.h>

#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/transform.h>
#include <math/vector.h>
#include <system/rtti.h>

namespace
{
template <typename T>
T RoundTrip( const T& value )
{
	Json::Value json;
	EXPECT_TRUE( rtti::ValueToJson( value, json ) );

	T result;
	EXPECT_TRUE( rtti::ValueFromJson( json, result ) );
	return result;
}

template <typename T>
void ExpectRejectsMissingMember( const T& value, const char* memberName )
{
	Json::Value json;
	ASSERT_TRUE( rtti::ValueToJson( value, json ) );
	json.removeMember( memberName );

	T result;
	EXPECT_FALSE( rtti::ValueFromJson( json, result ) ) << "Missing member: " << memberName;
}

template <typename T>
void ExpectRejectsNonObject()
{
	T result;
	EXPECT_FALSE( rtti::ValueFromJson( Json::Value( 5 ), result ) );
	EXPECT_FALSE( rtti::ValueFromJson( Json::Value( Json::arrayValue ), result ) );
}
} // namespace

TEST( SerializationTypesTests, Vec2RoundTrip )
{
	const Vec2 value( 1.25f, -2.5f );

	EXPECT_TRUE( RoundTrip( value ) == value );
	ExpectRejectsMissingMember( value, "x" );
	ExpectRejectsMissingMember( value, "y" );
	ExpectRejectsNonObject<Vec2>();
}

TEST( SerializationTypesTests, Vec3RoundTrip )
{
	const Vec3 value( 0.1f, -0.2f, 0.3f );

	EXPECT_TRUE( RoundTrip( value ) == value );
	ExpectRejectsMissingMember( value, "z" );
	ExpectRejectsNonObject<Vec3>();
}

TEST( SerializationTypesTests, Vec4RoundTrip )
{
	const Vec4 value( 1.0f, 2.0f, 3.0f, 4.0f );

	EXPECT_TRUE( RoundTrip( value ) == value );
	ExpectRejectsMissingMember( value, "w" );
	ExpectRejectsNonObject<Vec4>();
}

TEST( SerializationTypesTests, VectorRejectsNonNumericComponents )
{
	Json::Value json( Json::objectValue );
	json["x"] = "not a number";
	json["y"] = 1.0f;

	Vec2 result;
	EXPECT_FALSE( rtti::ValueFromJson( json, result ) );
}

TEST( SerializationTypesTests, QuatRoundTrip )
{
	const Quat value( 0.1f, 0.2f, 0.3f, 0.9f );
	const Quat result = RoundTrip( value );

	EXPECT_EQ( result.x, value.x );
	EXPECT_EQ( result.y, value.y );
	EXPECT_EQ( result.z, value.z );
	EXPECT_EQ( result.w, value.w );
	ExpectRejectsMissingMember( value, "w" );
	ExpectRejectsNonObject<Quat>();
}

TEST( SerializationTypesTests, Matrix33RoundTripKeepsRowOrder )
{
	Matrix33 value;
	value.SetRow( 0, Vec3( 1.0f, 2.0f, 3.0f ) );
	value.SetRow( 1, Vec3( 4.0f, 5.0f, 6.0f ) );
	value.SetRow( 2, Vec3( 7.0f, 8.0f, 9.0f ) );

	const Matrix33 result = RoundTrip( value );

	EXPECT_TRUE( result.GetRow( 0 ) == value.GetRow( 0 ) );
	EXPECT_TRUE( result.GetRow( 1 ) == value.GetRow( 1 ) );
	EXPECT_TRUE( result.GetRow( 2 ) == value.GetRow( 2 ) );
	ExpectRejectsMissingMember( value, "row2" );
	ExpectRejectsNonObject<Matrix33>();
}

TEST( SerializationTypesTests, Matrix44RoundTripKeepsRowOrder )
{
	Matrix44 value;
	value.SetRow( 0, Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	value.SetRow( 1, Vec4( 5.0f, 6.0f, 7.0f, 8.0f ) );
	value.SetRow( 2, Vec4( 9.0f, 10.0f, 11.0f, 12.0f ) );
	value.SetRow( 3, Vec4( 13.0f, 14.0f, 15.0f, 16.0f ) );

	const Matrix44 result = RoundTrip( value );

	EXPECT_TRUE( result.GetRow( 0 ) == value.GetRow( 0 ) );
	EXPECT_TRUE( result.GetRow( 1 ) == value.GetRow( 1 ) );
	EXPECT_TRUE( result.GetRow( 2 ) == value.GetRow( 2 ) );
	EXPECT_TRUE( result.GetRow( 3 ) == value.GetRow( 3 ) );
	ExpectRejectsMissingMember( value, "row3" );
	ExpectRejectsNonObject<Matrix44>();
}

TEST( SerializationTypesTests, TransformRoundTrip )
{
	const Transform value( Vec3( 1.0f, 2.0f, 3.0f ), Quat( 0.0f, 0.6f, 0.0f, 0.8f ), Vec3( 2.0f, 3.0f, 4.0f ) );
	const Transform result = RoundTrip( value );

	EXPECT_TRUE( result.position == value.position );
	EXPECT_EQ( result.rotation.y, value.rotation.y );
	EXPECT_EQ( result.rotation.w, value.rotation.w );
	EXPECT_TRUE( result.scale == value.scale );
	ExpectRejectsMissingMember( value, "position" );
	ExpectRejectsMissingMember( value, "rotation" );
	ExpectRejectsMissingMember( value, "scale" );
	ExpectRejectsNonObject<Transform>();
}
