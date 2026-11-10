#include <gaia3.h>

#include <components/cameraComponent.h>
#include <components/lightComponent.h>
#include <components/modelComponent.h>

#include <charconv>

namespace
{
struct SandboxOptions
{
	bool        bNoEditor = false;
	bool        bNoDock   = false;
	u64         maxFrames = 0;
	std::string level;
};

SandboxOptions ParseArguments( int argc, char** argv )
{
	SandboxOptions options;

	for( int i = 1; i < argc; ++i )
	{
		const std::string_view argument = argv[i];
		if( argument == "-no_editor" )
		{
			options.bNoEditor = true;
		}
		else if( argument == "-no_dock" )
		{
			options.bNoDock = true;
		}
		else if( argument == "-frames" && i + 1 < argc )
		{
			const std::string_view value = argv[++i];

			const auto [parsedEnd, errorCode] = std::from_chars( value.data(), value.data() + value.size(), options.maxFrames );
			if( errorCode != std::errc() || parsedEnd != value.data() + value.size() )
			{
				GAIA_WARN( "Sandbox: invalid frame count {}, running until closed", value );
				options.maxFrames = 0;
			}
		}
		else if( argument == "-level" && i + 1 < argc )
		{
			options.level = argv[++i];
		}
		else
		{
			GAIA_WARN( "Sandbox: unknown argument {}", argument );
		}
	}

	return options;
}

void createDefaultLevel( Level* pLevel )
{
	// Create camera
	Entity* pCamera = pLevel->CreateEntity( "Camera" );
	{
		pCamera->SetPosition( Vec3( 1.0f, 1.0f, -4.12f ) );
		pCamera->SetRotation( LookAtQuat( Vec3( 0.0f, 0.0f, 1.0f ), Vec3::V_Up ) );
		pCamera->AddComponent<CameraComponent>();
	}

	// Create cube in the middle
	Entity* pCube = pLevel->CreateEntity( "Cube" );
	{
		shared<ModelComponent> pModelComp = pCube->AddComponent<ModelComponent>();
		pModelComp->SetModel( "cube" );
	}

	// Create the sun, shining down at an angle
	Entity* pSun = pLevel->CreateEntity( "Sun" );
	{
		pSun->SetRotation( LookAtQuat( Vec3( -0.4f, -0.8f, -0.6f ).Normalized(), Vec3::V_Up ) );

		shared<DirectionalLightComponent> pSunLight = pSun->AddComponent<DirectionalLightComponent>();
		pSunLight->SetColor( Vec3( 1.0f, 0.95f, 0.85f ) );
		pSunLight->SetIntensity( 1.0f );
	}

	// Create a warm point light next to the cube, lighting its left side
	Entity* pLamp = pLevel->CreateEntity( "Point Light" );
	{
		pLamp->SetPosition( Vec3( 1.5f, 0.5f, -1.0f ) );

		shared<PointLightComponent> pLampLight = pLamp->AddComponent<PointLightComponent>();
		pLampLight->SetColor( Vec3( 1.0f, 0.55f, 0.25f ) );
		pLampLight->SetIntensity( 4.0f );
		pLampLight->SetRange( 5.0f );
	}
}
} // namespace

int main( int argc, char** argv )
{
	const SandboxOptions options = ParseArguments( argc, argv );

	AppConfig config;
	config.canvas     = { 2560, 1440 };
	config.title      = "Sandbox";
	config.startLevel = options.level;
	config.appPath    = SANDBOX_ASSETS_PATH;
	config.commonPath = GAIA3_ASSETS_PATH;

	config.maxFrames = options.maxFrames;

	config.editor.enabled = !options.bNoEditor;
	config.editor.docked  = !options.bNoDock;

	config.editor.gizmoMode = GizmoMode::Translate;

	if( !App::Init( config ) )
		return 1;

	// If New, create a default sandbox level
	{
		Level* pLevel = App::GetSession().GetCurrentLevel();
		GAIA_ASSERT( pLevel != nullptr );
		if( pLevel->IsNew() )
		{
			createDefaultLevel( pLevel );
		}
	}

	App::Run();
	App::Shutdown();
	return 0;
}