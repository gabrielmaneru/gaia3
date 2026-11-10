#include "gaia3_pch.h"
#include "rendererTypes.h"

#include <core/app.h>
#include <gfx/renderer.h>

void Renderable::Register()
{
	std::vector<Renderable*>& vecRenderables = App::GetRenderer().m_Renderables;

	if( std::find( vecRenderables.begin(), vecRenderables.end(), this ) == vecRenderables.end() )
		vecRenderables.push_back( this );
}

void Renderable::Unregister()
{
	std::vector<Renderable*>& vecRenderables = App::GetRenderer().m_Renderables;

	auto it = std::find( vecRenderables.begin(), vecRenderables.end(), this );
	if( it != vecRenderables.end() )
		vecRenderables.erase( it );
}

void RenderCamera::Register()
{
	std::vector<RenderCamera*>& vecCameras = App::GetRenderer().m_Cameras;

	if( std::find( vecCameras.begin(), vecCameras.end(), this ) == vecCameras.end() )
		vecCameras.push_back( this );
}

void RenderCamera::Unregister()
{
	std::vector<RenderCamera*>& vecCameras = App::GetRenderer().m_Cameras;

	auto it = std::find( vecCameras.begin(), vecCameras.end(), this );
	if( it != vecCameras.end() )
		vecCameras.erase( it );
}

void RenderLight::Register()
{
	std::vector<RenderLight*>& vecLights = App::GetRenderer().m_Lights;

	if( std::find( vecLights.begin(), vecLights.end(), this ) == vecLights.end() )
		vecLights.push_back( this );
}

void RenderLight::Unregister()
{
	std::vector<RenderLight*>& vecLights = App::GetRenderer().m_Lights;

	auto it = std::find( vecLights.begin(), vecLights.end(), this );
	if( it != vecLights.end() )
		vecLights.erase( it );
}
