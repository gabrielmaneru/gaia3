#include <gaia3.h>

#include <components/cameraComponent.h>
#include <components/modelComponent.h>

namespace
{
void createDefaultLevel( Level* pLevel )
{
	// Create camera
	Entity* pCamera = pLevel->CreateEntity( "Camera" );
	{
		pCamera->GetTransformRef().position = Vec3( 1.0f, 1.0f, -4.12f );
		pCamera->AddComponent<CameraComponent>();
	}

	// Create cube in the middle
	Entity* pCube = pLevel->CreateEntity( "Cube" );
	{
		pCube->AddComponent<ModelComponent>( "cube.obj" );
	}
}
} // namespace

int main()
{
	AppConfig config;
	config.canvas     = { 1920, 1080 };
	config.title      = "Sandbox";
	config.startLevel = "test.lvl";
	config.appPath    = R"(.\assets\)";
	config.commonPath = GAIA3_ASSETS_PATH;

	if( !App::Init( config ) )
		return 1;

	// If New, create a default sandbox level
	{
		Level* pLevel = App::GetSession().GetCurrentLevel();
		assert( pLevel != nullptr );
		if( pLevel->IsNew() )
		{
			createDefaultLevel( pLevel );
		}
	}

	App::Run();

	// If New, try to save it
	{
		Level* pLevel = App::GetSession().GetCurrentLevel();
		if( pLevel->IsNew() )
		{
			App::GetSession().SaveCurrentLevelAs( "test.lvl" );
		}
	}

	App::Shutdown();
	return 0;
}