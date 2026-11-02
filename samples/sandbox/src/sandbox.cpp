#include <gaia3.h>

void createDefaultLevel( Level* pLevel )
{
	// Create camera
	Entity* pCamera = pLevel->CreateEntity( "Camera" );
	{
	}

	// Create triangle in the middle
	Entity* pTriangle = pLevel->CreateEntity( "Triangle" );
	{
	}
}

int main()
{
	AppConfig config;
	config.canvas     = { 1920, 1080 };
	config.title      = "Sandbox";
	config.startLevel = "test.lvl";
	config.appPath    = R"(.\assets\)";

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