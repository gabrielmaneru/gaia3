#include "testHelpers.h"

#include <gfx/renderer.h>
#include <gfx/shadowMap.h>
#include <math/quaternionTransforms.h>

#include <math.h>

namespace
{
const Vec3      kSunDirection  = Vec3( -0.3f, -1.0f, -0.2f ).Normalized();
const Vec3      kLightPosition = Vec3( 3.0f, 2.0f, -1.0f );
constexpr float kSize          = 40.0f;
constexpr float kDepth         = 30.0f;
constexpr u32   kMapSize       = 2048;

// Note: The directions across the light that the shadow map's sides follow
Vec3 AcrossRight()
{
	return LookAtQuat( kSunDirection, Vec3::V_Up ).Rotate( Vec3::V_Right );
}

Vec3 AcrossUp()
{
	return LookAtQuat( kSunDirection, Vec3::V_Up ).Rotate( Vec3::V_Up );
}

Vec3 Project( const ShadowView& shadowView, const Vec3& point )
{
	const Vec4 clip = shadowView.viewProjection * Vec4( point.x, point.y, point.z, 1.0f );
	return Vec3( clip.x / clip.w, clip.y / clip.w, clip.z / clip.w );
}

bool IsInsideTheMap( const Vec3& clip )
{
	return fabsf( clip.x ) <= 1.0f && fabsf( clip.y ) <= 1.0f && fabsf( clip.z ) <= 1.0f;
}
} // namespace

TEST( ShadowMapTests, TheBoxIsInsideTheMap )
{
	const ShadowView shadowView = FitShadowToBox( kLightPosition, kSunDirection, kSize, kDepth, kMapSize );

	// Note: The box starts at the light, so its middle is half its depth along the light
	const Vec3 middle     = kLightPosition + kSunDirection * Scalar( kDepth * 0.5f );
	const Vec3 middleClip = Project( shadowView, middle );
	EXPECT_NEAR( middleClip.x, 0.0f, 1e-4f );
	EXPECT_NEAR( middleClip.y, 0.0f, 1e-4f );
	EXPECT_NEAR( middleClip.z, 0.0f, 1e-4f );

	const Vec3 lightClip = Project( shadowView, kLightPosition );
	EXPECT_NEAR( lightClip.z, -1.0f, 1e-4f );

	// Note: Just inside each side of the box
	EXPECT_TRUE( IsInsideTheMap( Project( shadowView, middle + AcrossRight() * Scalar( kSize * 0.49f ) ) ) );
	EXPECT_TRUE( IsInsideTheMap( Project( shadowView, middle - AcrossUp() * Scalar( kSize * 0.49f ) ) ) );
	EXPECT_TRUE( IsInsideTheMap( Project( shadowView, kLightPosition + kSunDirection * Scalar( kDepth * 0.01f ) ) ) );
	EXPECT_TRUE( IsInsideTheMap( Project( shadowView, kLightPosition + kSunDirection * Scalar( kDepth * 0.99f ) ) ) );
}

TEST( ShadowMapTests, OutsideTheBoxIsOutsideTheMap )
{
	const ShadowView shadowView = FitShadowToBox( kLightPosition, kSunDirection, kSize, kDepth, kMapSize );
	const Vec3       middle     = kLightPosition + kSunDirection * Scalar( kDepth * 0.5f );

	EXPECT_FALSE( IsInsideTheMap( Project( shadowView, middle + AcrossRight() * Scalar( kSize * 0.6f ) ) ) );
	EXPECT_FALSE( IsInsideTheMap( Project( shadowView, middle + AcrossUp() * Scalar( kSize * 0.6f ) ) ) );
	EXPECT_FALSE( IsInsideTheMap( Project( shadowView, kLightPosition + kSunDirection * Scalar( kDepth * 1.1f ) ) ) );

	// Note: Behind the light, which only counts for casters, their depth is clamped while drawing the shadow map
	EXPECT_FALSE( IsInsideTheMap( Project( shadowView, kLightPosition - kSunDirection * Scalar( 1.0f ) ) ) );
}
TEST( ShadowMapTests, TheMapLooksAlongTheLight )
{
	const ShadowView shadowView = FitShadowToBox( kLightPosition, kSunDirection, kSize, kDepth, kMapSize );

	// Note: Moving along the light only changes the depth, so a point and its shadow share a texel
	const Vec3 point  = Vec3( 5.0f, 1.0f, -4.0f );
	const Vec3 higher = Project( shadowView, point - kSunDirection * Scalar( 2.0f ) );
	const Vec3 lower  = Project( shadowView, point );

	EXPECT_NEAR( higher.x, lower.x, 1e-4f );
	EXPECT_NEAR( higher.y, lower.y, 1e-4f );
	EXPECT_LT( higher.z, lower.z );
}

TEST( ShadowMapTests, MovingTheLightMovesTheBox )
{
	const Vec3       offset = Vec3( 4.0f, -1.0f, 2.5f );
	const ShadowView before = FitShadowToBox( kLightPosition, kSunDirection, kSize, kDepth, kMapSize );
	const ShadowView after  = FitShadowToBox( kLightPosition + offset, kSunDirection, kSize, kDepth, kMapSize );

	const Vec3 point       = Vec3( 1.0f, 0.0f, -2.0f );
	const Vec3 beforeClip  = Project( before, point );
	const Vec3 movedClip   = Project( after, point + offset );

	EXPECT_NEAR( beforeClip.x, movedClip.x, 1e-4f );
	EXPECT_NEAR( beforeClip.y, movedClip.y, 1e-4f );
	EXPECT_NEAR( beforeClip.z, movedClip.z, 1e-4f );
}

TEST( ShadowMapTests, TexelsSplitTheBoxEvenly )
{
	EXPECT_FLOAT_EQ( FitShadowToBox( kLightPosition, kSunDirection, kSize, kDepth, kMapSize ).texelWorldSize, kSize / kMapSize );
	EXPECT_FLOAT_EQ( FitShadowToBox( kLightPosition, kSunDirection, kSize, kDepth, kMapSize * 2 ).texelWorldSize, kSize / ( kMapSize * 2 ) );
}

TEST( ShadowMapTests, AnEmptyBoxStillProjects )
{
	const ShadowView shadowView = FitShadowToBox( kLightPosition, kSunDirection, 0.0f, 0.0f, kMapSize );

	EXPECT_GT( shadowView.texelWorldSize, 0.0f );
	EXPECT_TRUE( IsInsideTheMap( Project( shadowView, kLightPosition ) ) );
}

TEST( ShadowMapTests, OnlyAShadowCastingDirectionalLightGetsTheMap )
{
	LightParameters lamp;
	lamp.type         = LightType::Point;
	lamp.bCastShadows = true;

	LightParameters moon;
	moon.type = LightType::Directional;

	LightParameters sun;
	sun.type         = LightType::Directional;
	sun.bCastShadows = true;

	EXPECT_EQ( Renderer::FindShadowLight( {} ), -1 );
	EXPECT_EQ( Renderer::FindShadowLight( { lamp, moon } ), -1 );
	EXPECT_EQ( Renderer::FindShadowLight( { lamp, moon, sun, sun } ), 2 );
}

TEST( ShadowMapTests, NoLightHasTheShadowMapBeforeOneIsDrawn )
{
	const Renderer renderer;

	EXPECT_EQ( renderer.GetShadowCaster(), nullptr );
	EXPECT_EQ( renderer.GetShadowMapTexture(), 0u );
}

TEST( ShadowMapTests, ShadowsStartOnWithTheDefaultSettings )
{
	const Renderer renderer;

	const ShadowSettings& settings = renderer.GetShadowSettings();
	EXPECT_TRUE( settings.bEnabled );
	EXPECT_EQ( settings.mapSize, 2048u );
	EXPECT_GT( settings.depthBias, 0.0f );
	EXPECT_GT( settings.normalBias, 0.0f );
}
