#include "testHelpers.h"

#include <gfx/rendererTypes.h>
#include <gfx/shader.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

TEST( ShaderTests, SplitsTheVertexAndFragmentStages )
{
	std::string vertex;
	std::string fragment;

	ASSERT_TRUE( Shader::SplitStages( "#type vertex\nvoid main() {}\n#type fragment\nout vec4 color;\n", vertex, fragment ) );

	EXPECT_EQ( vertex, "void main() {}\n" );
	EXPECT_EQ( fragment, "out vec4 color;\n" );
}

TEST( ShaderTests, LinesBeforeTheFirstStageAreSkipped )
{
	std::string vertex;
	std::string fragment;

	ASSERT_TRUE( Shader::SplitStages( "// shared comment\n#type fragment\nB\n#type vertex\nA\n", vertex, fragment ) );

	EXPECT_EQ( vertex, "A\n" );
	EXPECT_EQ( fragment, "B\n" );
}

TEST( ShaderTests, BothStagesAreRequired )
{
	std::string vertex;
	std::string fragment;

	EXPECT_FALSE( Shader::SplitStages( "#type vertex\nA\n", vertex, fragment ) );
	EXPECT_FALSE( Shader::SplitStages( "#type fragment\nB\n", vertex, fragment ) );
	EXPECT_FALSE( Shader::SplitStages( "no stages at all\n", vertex, fragment ) );
}

TEST( ShaderTests, UnknownStagesAreReported )
{
	LogCapture  log;
	std::string vertex;
	std::string fragment;

	EXPECT_FALSE( Shader::SplitStages( "#type vertex\nA\n#type geometry\nC\n#type fragment\nB\n", vertex, fragment ) );
	EXPECT_TRUE( log.Contains( "error: Shader: unknown stage geometry" ) );
}

namespace
{
// Note: Includes from memory, so the tests need no files
Shader::IncludeReader MakeReader( std::map<std::string, std::string> includes )
{
	return [includes]( std::string_view includeName, std::string& outSource ) {
		const auto includeIt = includes.find( std::string( includeName ) );
		if( includeIt == includes.end() )
			return false;

		outSource = includeIt->second;
		return true;
	};
}

std::string ReadEngineFile( const std::string& relativePath )
{
	std::ifstream     file( GAIA3_ASSETS_PATH + relativePath );
	std::stringstream text;
	text << file.rdbuf();
	return text.str();
}
} // namespace

TEST( ShaderTests, IncludesArePastedWithLineMarks )
{
	std::string              source;
	std::vector<std::string> sourceNames;
	ASSERT_TRUE( Shader::ResolveIncludes( "#version 330 core\n#include \"math.glsli\"\nvoid main() {}\n", MakeReader( { { "math.glsli", "float Twice( float x ) { return 2.0 * x; }\n" } } ), source, sourceNames ) );

	// Note: The include starts at its line 1 as source 1, then the main file goes on at line 3 as source 0
	EXPECT_EQ( source, "#version 330 core\n#line 1 1\nfloat Twice( float x ) { return 2.0 * x; }\n#line 3 0\nvoid main() {}\n" );
	EXPECT_EQ( sourceNames, ( std::vector<std::string>{ "main", "math.glsli" } ) );
}

TEST( ShaderTests, IncludesCanIncludeOthersAndArePastedOnce )
{
	const Shader::IncludeReader reader = MakeReader( {
	    { "lighting.glsli", "#include \"math.glsli\"\nvec3 Light() { return vec3( Twice( 1.0 ) ); }\n" },
	    { "math.glsli", "float Twice( float x ) { return 2.0 * x; }\n" },
	} );

	std::string              source;
	std::vector<std::string> sourceNames;
	ASSERT_TRUE( Shader::ResolveIncludes( "#include \"math.glsli\"\n#include \"lighting.glsli\"\n", reader, source, sourceNames ) );

	EXPECT_EQ( sourceNames, ( std::vector<std::string>{ "main", "math.glsli", "lighting.glsli" } ) );

	// Note: math.glsli is pasted by the main file, lighting.glsli's own #include of it becomes an empty line
	const u64 first = source.find( "float Twice" );
	ASSERT_NE( first, std::string::npos );
	EXPECT_EQ( source.find( "float Twice", first + 1 ), std::string::npos );
	EXPECT_NE( source.find( "vec3 Light()" ), std::string::npos );
	EXPECT_EQ( source.find( "#include" ), std::string::npos );
}

TEST( ShaderTests, IncludesThatIncludeEachOtherStop )
{
	const Shader::IncludeReader reader = MakeReader( {
	    { "a.glsli", "#include \"b.glsli\"\nfloat A;\n" },
	    { "b.glsli", "#include \"a.glsli\"\nfloat B;\n" },
	} );

	std::string              source;
	std::vector<std::string> sourceNames;
	ASSERT_TRUE( Shader::ResolveIncludes( "#include \"a.glsli\"\n", reader, source, sourceNames ) );
	EXPECT_EQ( sourceNames.size(), 3u );
}

TEST( ShaderTests, MissingOrBadIncludesFail )
{
	LogCapture               log;
	std::string              source;
	std::vector<std::string> sourceNames;

	EXPECT_FALSE( Shader::ResolveIncludes( "void main() {}\n#include \"missing.glsli\"\n", MakeReader( {} ), source, sourceNames ) );
	EXPECT_TRUE( log.Contains( "error: Shader: main line 2: include missing.glsli is not available" ) );

	EXPECT_FALSE( Shader::ResolveIncludes( "#include <pbr.glsli>\n", MakeReader( {} ), source, sourceNames ) );
	EXPECT_TRUE( log.Contains( "an include needs its name in quotes" ) );
}

TEST( ShaderTests, TheDefaultShaderIncludesTheEnginePbrCode )
{
	std::string vertex;
	std::string fragment;
	ASSERT_TRUE( Shader::SplitStages( ReadEngineFile( "shaders/default.glsl" ), vertex, fragment ) );

	auto readEngineInclude = []( std::string_view includeName, std::string& outSource ) {
		outSource = ReadEngineFile( "shaders/" + std::string( includeName ) );
		return !outSource.empty();
	};

	std::string              source;
	std::vector<std::string> sourceNames;
	ASSERT_TRUE( Shader::ResolveIncludes( fragment, readEngineInclude, source, sourceNames ) );
	EXPECT_NE( std::find( sourceNames.begin(), sourceNames.end(), "pbr.glsli" ), sourceNames.end() );
	EXPECT_NE( source.find( "vec3 GetPbrLighting(" ), std::string::npos );

	// Note: #version has to stay the first line
	EXPECT_EQ( source.find( "#version 330 core" ), 0u );
}

TEST( ShaderTests, HandlesWindowsLineEndings )
{
	std::string vertex;
	std::string fragment;

	ASSERT_TRUE( Shader::SplitStages( "#type vertex\r\nA\r\n#type fragment\r\nB\r\n", vertex, fragment ) );

	EXPECT_NE( vertex.find( 'A' ), std::string::npos );
	EXPECT_NE( fragment.find( 'B' ), std::string::npos );
}

TEST( ShaderTests, TheEngineDefaultShaderHasBothStages )
{
	std::ifstream file( GAIA3_ASSETS_PATH "shaders/default.glsl" );
	ASSERT_TRUE( file.is_open() );

	std::stringstream text;
	text << file.rdbuf();

	std::string vertex;
	std::string fragment;
	ASSERT_TRUE( Shader::SplitStages( text.str(), vertex, fragment ) );

	EXPECT_EQ( vertex.find( "#version 330 core" ), 0u );
	EXPECT_EQ( fragment.find( "#version 330 core" ), 0u );
	EXPECT_NE( fragment.find( "u_Color" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_Texture" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_AlphaTest" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_OpacityTexture" ), std::string::npos );
	EXPECT_NE( fragment.find( "discard" ), std::string::npos );

	// Note: The shader's light limit must match the renderer's
	EXPECT_NE( fragment.find( "const int kMaxLights        = " + std::to_string( kMaxLights ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_LightColor" ), std::string::npos );

	// Note: The tangent attribute must match kTangentAttribute in gfx/model.cpp
	EXPECT_NE( vertex.find( "layout( location = 3 ) in vec4 aTangent;" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_NormalTexture" ), std::string::npos );

	// Note: Two-sided materials light their back faces with the normal flipped
	EXPECT_NE( fragment.find( "gl_FrontFacing" ), std::string::npos );

	// Note: The draw modes must match DrawMode
	EXPECT_NE( fragment.find( "const int kDrawLit       = " + std::to_string( static_cast<int>( DrawMode::Lit ) ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "const int kDrawNormals   = " + std::to_string( static_cast<int>( DrawMode::Normals ) ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "const int kDrawShadows   = " + std::to_string( static_cast<int>( DrawMode::Shadows ) ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "const int kDrawRoughness = " + std::to_string( static_cast<int>( DrawMode::Roughness ) ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "const int kDrawMetallic  = " + std::to_string( static_cast<int>( DrawMode::Metallic ) ) + ";" ), std::string::npos );

	// Note: The names MaterialEntry::Bind sets for PBR
	EXPECT_NE( fragment.find( "u_CameraPosition" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_Roughness" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_RoughnessTexture" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_Metallic" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_MetallicTexture" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_DrawMode" ), std::string::npos );

	// Note: The names MaterialEntry::Bind sets for the sun's shadow
	EXPECT_NE( fragment.find( "uniform sampler2DShadow u_ShadowMap;" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_ShadowLight" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_ShadowMatrix" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_ShadowDepthBias" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_ShadowNormalOffset" ), std::string::npos );
}

TEST( ShaderTests, TheEngineShadowShaderHasBothStages )
{
	std::ifstream file( GAIA3_ASSETS_PATH "shaders/shadow.glsl" );
	ASSERT_TRUE( file.is_open() );

	std::stringstream text;
	text << file.rdbuf();

	std::string vertex;
	std::string fragment;
	ASSERT_TRUE( Shader::SplitStages( text.str(), vertex, fragment ) );

	// Note: The attributes must match gfx/model.cpp, the uniforms MaterialEntry::BindShadow
	EXPECT_NE( vertex.find( "layout( location = 0 ) in vec3 aPosition;" ), std::string::npos );
	EXPECT_NE( vertex.find( "layout( location = 2 ) in vec2 aUV;" ), std::string::npos );
	EXPECT_NE( vertex.find( "u_ShadowMatrix" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_AlphaTest" ), std::string::npos );
	EXPECT_NE( fragment.find( "discard" ), std::string::npos );
}
