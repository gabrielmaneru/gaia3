#pragma once

#include <core/types.h>
#include <math/matrix.h>

// Note: What the renderer hands to each renderable when it draws
struct DrawContext
{
	Matrix44 model;
	Matrix44 view;
	Matrix44 projection;
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