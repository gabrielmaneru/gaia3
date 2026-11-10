#include "testHelpers.h"

#include <gfx/rendererTypes.h>
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
	EXPECT_NE( fragment.find( "const int kDrawLit     = " + std::to_string( static_cast<int>( DrawMode::Lit ) ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "const int kDrawNormals = " + std::to_string( static_cast<int>( DrawMode::Normals ) ) + ";" ), std::string::npos );
	EXPECT_NE( fragment.find( "u_DrawMode" ), std::string::npos );
}
