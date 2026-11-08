#include "testHelpers.h"

#include "scene/testComponents.h"

#include <core/session.h>
#include <gfx/objImporter.h>
#include <math/mathHelpers.h>
#include <math/matrixTransforms.h>
#include <math/transform.h>
#include <system/resourceManager.h>

TEST( WarningLogTests, NormalizingZeroVectorsKeepsZeroAndWarns )
{
	LogCapture log;

	EXPECT_TRUE( Vec2::V_Zero.Normalized() == Vec2::V_Zero );
	EXPECT_TRUE( log.Contains( "warning: Vec2" ) );

	EXPECT_TRUE( Vec3::V_Zero.Normalized() == Vec3::V_Zero );
	EXPECT_TRUE( log.Contains( "warning: Vec3" ) );

	EXPECT_TRUE( Vec4::V_Zero.Normalized() == Vec4::V_Zero );
	EXPECT_TRUE( log.Contains( "warning: Vec4" ) );
}

TEST( WarningLogTests, NormalizingValidVectorsDoesNotWarn )
{
	LogCapture log;

	Vec3( 1.0f, 2.0f, 3.0f ).Normalized();
	Quat( 0.1f, 0.2f, 0.3f, 0.9f ).Normalized();

	EXPECT_TRUE( log.GetOutput().empty() );
}

TEST( WarningLogTests, NormalizingAZeroQuaternionUsesIdentityAndWarns )
{
	LogCapture log;

	const Quat result = Quat::V_Zero.Normalized();

	EXPECT_FLOAT_EQ( result.w, 1.0f );
	EXPECT_TRUE( log.Contains( "warning: Quat" ) );
}

TEST( WarningLogTests, InvertingSingularMatricesUsesIdentityAndWarns )
{
	LogCapture log;

	const Matrix33 singular33( Vec3::V_Zero, Vec3::V_Zero, Vec3::V_Zero );
	const Matrix44 singular44( Vec4::V_Zero, Vec4::V_Zero, Vec4::V_Zero, Vec4::V_Zero );

	const Matrix33 inverse33 = singular33.Inverse();
	const Matrix44 inverse44 = singular44.Inverse();

	EXPECT_FLOAT_EQ( inverse33.m00, 1.0f );
	EXPECT_FLOAT_EQ( inverse33.m01, 0.0f );
	EXPECT_FLOAT_EQ( inverse44.m00, 1.0f );
	EXPECT_FLOAT_EQ( inverse44.m33, 1.0f );
	EXPECT_TRUE( log.Contains( "warning: Matrix33" ) );
	EXPECT_TRUE( log.Contains( "warning: Matrix44" ) );
}

TEST( WarningLogTests, InvertingAZeroScaleTransformUsesIdentityAndWarns )
{
	LogCapture log;

	const Transform zeroScale( Vec3( 1.0f, 2.0f, 3.0f ), Quat::V_Identity, Vec3( 0.0f, 1.0f, 1.0f ) );
	const Transform result = zeroScale.Inverse();

	EXPECT_TRUE( result.position == Vec3::V_Zero );
	EXPECT_TRUE( result.scale == Vec3::V_One );
	EXPECT_TRUE( log.Contains( "warning: Transform" ) );
}

TEST( WarningLogTests, PerspectiveWarnsOnlyForInvalidArguments )
{
	LogCapture log;

	Perspective( 1.0f, 1.5f, 0.1f, 100.0f );
	EXPECT_TRUE( log.GetOutput().empty() );

	Perspective( 1.0f, 0.0f, 0.1f, 100.0f );
	EXPECT_TRUE( log.Contains( "warning: Perspective" ) );
}

TEST( WarningLogTests, RampWarnsForAnEmptyRange )
{
	LogCapture log;

	EXPECT_FLOAT_EQ( Ramp( 1.0f, 2.0f, 2.0f ), 0.0f );
	EXPECT_TRUE( log.Contains( "warning: Ramp" ) );
}

TEST( WarningLogTests, MeshWithoutNormalsLoadsAndWarns )
{
	ScopedTempDir directory;
	LogCapture    log;

	ModelData data;
	EXPECT_TRUE( ObjImporter::Import( directory.Write( "mesh.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n" ), data ) );

	EXPECT_TRUE( log.Contains( "has no normals" ) );
}

TEST( WarningLogTests, MeshWithNormalsDoesNotWarn )
{
	ScopedTempDir directory;
	LogCapture    log;

	ModelData data;
	EXPECT_TRUE( ObjImporter::Import( directory.Write( "mesh.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//1 2//1 3//1\n" ), data ) );

	EXPECT_FALSE( log.Contains( "warning" ) );
}

TEST( WarningLogTests, MissingResourceFolderIsSkippedWithAWarning )
{
	ScopedTempDir directory;
	LogCapture    log;

	ResourceManager manager;

	EXPECT_TRUE( manager.Init( "", directory.PathTo( "missing" ) ) );
	EXPECT_TRUE( log.Contains( "warning: ResourceManager" ) );
}

TEST( WarningLogTests, MissingStartLevelFallsBackWithAWarning )
{
	ScopedTempDir directory;
	LogCapture    log;

	Session session;

	EXPECT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );
	EXPECT_TRUE( log.Contains( "warning: Session: start level start.lvl" ) );
}

TEST( WarningLogTests, EmptyStartLevelCreatesANewLevelWithoutWarning )
{
	ScopedTempDir directory;
	LogCapture    log;

	Session session;

	ASSERT_TRUE( session.Init( "", "", directory.GetPath() ) );

	ASSERT_NE( session.GetCurrentLevel(), nullptr );
	EXPECT_TRUE( session.GetCurrentLevel()->IsNew() );
	EXPECT_FALSE( log.Contains( "warning" ) );
}

TEST( InfoLogTests, ResourceScanReportsWhatItFound )
{
	ScopedTempDir directory;
	directory.Write( "first.lvl", "{}" );
	directory.Write( "second.lvl", "{}" );
	LogCapture log;

	ResourceManager manager;
	ASSERT_TRUE( manager.Init( "", directory.GetPath() ) );

	EXPECT_TRUE( log.Contains( "info: ResourceManager: found 2 resources" ) );
}

TEST( InfoLogTests, LoadingALevelReportsTheResourceAndEachEntity )
{
	ScopedTempDir     directory;
	const std::string path = directory.PathTo( "town.lvl" );
	{
		Level   saved;
		Entity* player = saved.CreateEntity( "Player" );
		player->AddComponent<HealthComponent>();
		player->AddComponent<ArmorComponent>();
		saved.CreateEntity( "Empty" );
		ASSERT_TRUE( saved.SaveAs( path ) );
	}

	LogCapture log;

	Level level( path );
	ASSERT_TRUE( level.Load() );

	EXPECT_TRUE( log.Contains( "info: Level town: 2 entities" ) );
	EXPECT_TRUE( log.Contains( "Entity Player: 2 components" ) );
	EXPECT_TRUE( log.Contains( "Entity Empty: 0 components" ) );
	EXPECT_TRUE( log.Contains( "info: Loaded resource " + path ) );
}

TEST( InfoLogTests, SavingALevelReportsTheResource )
{
	ScopedTempDir directory;
	LogCapture    log;

	Level level;
	ASSERT_TRUE( level.SaveAs( directory.PathTo( "saved.lvl" ) ) );

	EXPECT_TRUE( log.Contains( "info: Saved resource" ) );
}

TEST( ErrorLogTests, LoadingAMissingLevelReportsTheFailure )
{
	ScopedTempDir directory;
	LogCapture    log;

	Level level( directory.PathTo( "missing.lvl" ) );

	EXPECT_FALSE( level.Load() );
	EXPECT_TRUE( log.Contains( "error: Level missing: failed to open" ) );
	EXPECT_TRUE( log.Contains( "error: Failed to load resource" ) );
	EXPECT_FALSE( log.Contains( "info:" ) );
}

TEST( ErrorLogTests, LoadingInvalidJsonReportsTheParserError )
{
	ScopedTempDir directory;
	LogCapture    log;

	Level level( directory.Write( "bad.lvl", "{ not json" ) );

	EXPECT_FALSE( level.Load() );
	EXPECT_TRUE( log.Contains( "error: Level bad: invalid JSON" ) );
}

TEST( ErrorLogTests, UnknownComponentTypesAreNamed )
{
	ScopedTempDir directory;
	LogCapture    log;

	Level level( directory.Write( "bad.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Hero",
			"m_SerializedComponents": [ { "m_TypeName": "NoSuchComponent", "m_Data": {} } ]
		} ]
	})" ) );

	EXPECT_FALSE( level.Load() );
	EXPECT_TRUE( log.Contains( "error: Entity Hero: unknown component type NoSuchComponent" ) );
	EXPECT_TRUE( log.Contains( "error: Level bad: failed to restore the components of entity Hero" ) );
}

TEST( ErrorLogTests, InvalidPropertiesAreNamed )
{
	ScopedTempDir directory;
	LogCapture    log;

	Level level( directory.Write( "bad.lvl", R"({
		"m_Entities": [ {
			"m_Name": "Hero",
			"m_SerializedComponents": [ { "m_TypeName": "HealthComponent", "m_Data": { "m_Health": "lots" } } ]
		} ]
	})" ) );

	EXPECT_FALSE( level.Load() );
	EXPECT_TRUE( log.Contains( "error: RTTI: invalid value for HealthComponent::m_Health" ) );
}

TEST( ErrorLogTests, BadStartLevelIsSkippedAndTheSessionContinues )
{
	ScopedTempDir directory;
	directory.Write( "start.lvl", "not json" );
	LogCapture log;

	Session session;

	EXPECT_TRUE( session.Init( "start.lvl", "", directory.GetPath() ) );
	EXPECT_TRUE( session.GetCurrentLevel()->IsNew() );
	EXPECT_TRUE( log.Contains( "error: Level start: invalid JSON" ) );
	EXPECT_TRUE( log.Contains( "warning: Session: start level" ) );
}

TEST( ErrorLogTests, InvalidMeshesReportTheLine )
{
	ScopedTempDir directory;
	LogCapture    log;

	const std::string path = directory.Write( "bad.obj", "v 0 0 0\nv 0 0\n" );
	ModelData         data;

	EXPECT_FALSE( ObjImporter::Import( path, data ) );
	EXPECT_TRUE( log.Contains( "error: ObjImporter: invalid data at " + path + ":2" ) );
}
