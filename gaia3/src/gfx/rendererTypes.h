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

class Shader;

// Note: What the default shader outputs. Normals shows each pixel's world-space normal, after normal mapping, as a
// colour: x to red, y to green and z to blue, each from -1..1 into 0..1. Shadows shows how much of the sun reaches
// each pixel, white when lit and black in shadow. Roughness and Metallic show those PBR values in grey, from black at
// 0 to white at 1. The values must match shaders/default.glsl
enum class DrawMode : int
{
	Lit       = 0,
	Normals   = 1,
	Shadows   = 2,
	Roughness = 3,
	Metallic  = 4
};

// Note: The sun's shadow map, the area it covers is set on the light. The biases keep a surface from shadowing itself:
// depthBias is in the shadow map's 0..1 depth, normalBias moves each point that many texels out along its surface's
// normal before looking it up
struct ShadowSettings
{
	bool  bEnabled   = true;
	u32   mapSize    = 2048;
	float depthBias  = 0.0005f;
	float normalBias = 1.5f;
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

	// Note: Only the first directional light that casts shadows gets them for now. Its shadow map covers a box that
	// starts at position, shadowSize wide and tall across the light, and runs shadowDepth along it
	bool  bCastShadows = false;
	float shadowSize   = 0.0f;
	float shadowDepth  = 0.0f;
};

// Note: What the renderer hands to each renderable when it draws
struct DrawContext
{
	Matrix44 model;
	Matrix44 view;
	Matrix44 projection;

	// Note: Where the camera is in the world, shiny surfaces reflect light towards it
	Vec3 cameraPosition;

	std::vector<LightParameters> lights;

	DrawMode drawMode = DrawMode::Lit;

	// Note: When false, cut-out materials draw their whole faces, showing the cards behind leaves and chains
	bool bAlphaTest = true;

	// Note: The sun's shadow map, when one was drawn this frame. shadowLight is its light's index in lights, -1 without
	// one. shadowMatrix takes world space to the shadow map, and shadowNormalOffset is normalBias in world units
	int      shadowLight = -1;
	Matrix44 shadowMatrix;
	u32      shadowMap          = 0;
	float    shadowDepthBias    = 0.0f;
	float    shadowNormalOffset = 0.0f;
};

class Renderable
{
public:
	virtual Matrix44 GetModelMatrix() const                   = 0;
	virtual void     Draw( const DrawContext& context ) const = 0;

	// Note: Draws only depth into the shadow map, seen through context.shadowMatrix. Renderables that cast no shadow
	// keep the default
	virtual void DrawShadow( const DrawContext& context, const Shader& shadowShader ) const {}

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
