#pragma once

#include <core/types.h>
#include <math/matrix.h>
#include <math/vector.h>

#include <vector>

// Note: Must match kMaxLights in shaders/default.glsl, extra lights are ignored
constexpr u32 kMaxLights = 8;

// Note: The values must match the light types in shaders/default.glsl
enum class LightType : int
{
	Directional = 0,
	Point       = 1
};

// Note: What the default shader outputs. Normals shows each pixel's world-space normal, after normal mapping, as a
// colour: x to red, y to green and z to blue, each from -1..1 into 0..1. The values must match shaders/default.glsl
enum class DrawMode : int
{
	Lit     = 0,
	Normals = 1
};

// Note: What the renderer needs to know about one light, in world space
struct LightParameters
{
	LightType type = LightType::Point;

	// Note: The light's colour already multiplied by its intensity
	Vec3 color = Vec3( 1.0f, 1.0f, 1.0f );

	Vec3  position;
	Vec3  direction = Vec3( 0.0f, 0.0f, -1.0f );
	float range     = 0.0f;
};

// Note: What the renderer hands to each renderable when it draws
struct DrawContext
{
	Matrix44 model;
	Matrix44 view;
	Matrix44 projection;

	std::vector<LightParameters> lights;

	DrawMode drawMode = DrawMode::Lit;

	// Note: When false, cut-out materials draw their whole faces, showing the cards behind leaves and chains
	bool bAlphaTest = true;
};

class Renderable
{
public:
	virtual Matrix44 GetModelMatrix() const                   = 0;
	virtual void     Draw( const DrawContext& context ) const = 0;

protected:
	virtual ~Renderable() = default;

	void Register();
	void Unregister();
};

class RenderCamera
{
public:
	virtual Matrix44 GetProjectionMatrix( rect canvas ) const = 0;
	virtual Matrix44 GetViewMatrix() const                    = 0;

protected:
	virtual ~RenderCamera() = default;

	void Register();
	void Unregister();
};

class RenderLight
{
public:
	virtual LightParameters GetLightParameters() const = 0;

protected:
	virtual ~RenderLight() = default;

	void Register();
	void Unregister();
};
