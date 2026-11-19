#include <gaia3.h>

#include "modelEditor.h"

#include <charconv>

namespace
{
struct ModelEditorOptions
{
	std::string projectPath = DEFAULT_PROJECT_PATH;
	std::string model;
	u64         maxFrames = 0;
};

ModelEditorOptions ParseArguments( int argc, char** argv )
{
	ModelEditorOptions options;

	for( int i = 1; i < argc; ++i )
	{
		const std::string_view argument = argv[i];
		if( argument == "-project" && i + 1 < argc )
		{
			options.projectPath = argv[++i];
		}
		else if( argument == "-model" && i + 1 < argc )
		{
			options.model = argv[++i];
		}
		else if( argument == "-frames" && i + 1 < argc )
		{
			const std::string_view value = argv[++i];

			const auto [parsedEnd, errorCode] = std::from_chars( value.data(), value.data() + value.size(), options.maxFrames );
			if( errorCode != std::errc() || parsedEnd != value.data() + value.size() )
			{
				GAIA_WARN( "Model Editor: invalid frame count {}, running until closed", value );
				options.maxFrames = 0;
			}
		}
		else
		{
			GAIA_WARN( "Model Editor: unknown argument {}", argument );
		}
	}

	return options;
}
} // namespace

int main( int argc, char** argv )
{
	const ModelEditorOptions options = ParseArguments( argc, argv );

	AppConfig config;
	config.canvas     = { 2560, 1440 };
	config.title      = "Model Editor";
	config.appPath    = options.projectPath;
	config.commonPath = GAIA3_ASSETS_PATH;
	config.maxFrames  = options.maxFrames;

	// Note: Always the docked editor, it is the whole app
	config.editor.enabled = true;
	config.editor.docked  = true;
	config.createEditor   = []( const EditorConfig& editorConfig ) -> unique<EditorShell> {
		return new_unique<ModelEditor>( editorConfig );
	};

	if( !App::Init( config ) )
		return 1;

	if( !options.model.empty() )
	{
		if( ModelEditor* editor = dynamic_cast<ModelEditor*>( App::GetEditor() ) )
		{
			editor->OpenModel( options.model );
		}
	}

	App::Run();
	App::Shutdown();
	return 0;
}
