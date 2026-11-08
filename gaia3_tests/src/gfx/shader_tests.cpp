#include "testHelpers.h"

#include <gfx/shader.h>

#include <fstream>
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
}
