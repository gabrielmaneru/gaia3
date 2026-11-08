#include "gaia3_pch.h"
#include "shader.h"

#include <glad/glad.h>

#include <fstream>
#include <sstream>

RTTI_DEFINE_RESOURCE( Shader,
                      RTTI_RESOURCE_FOLDER( "shaders" ),
                      RTTI_RESOURCE_EXTENSIONS( ".glsl" ) )

namespace
{
u32 CompileStage( u32 type, const std::string& source, std::string_view shaderName )
{
	const char* sourceText = source.c_str();

	const u32 stage = glCreateShader( type );
	glShaderSource( stage, 1, &sourceText, nullptr );
	glCompileShader( stage );

	int compileResult = 0;
	glGetShaderiv( stage, GL_COMPILE_STATUS, &compileResult );
	if( compileResult == GL_FALSE )
	{
		char logBuffer[512] = {};
		glGetShaderInfoLog( stage, sizeof( logBuffer ), nullptr, logBuffer );
		GAIA_ERROR( "Shader {}: {} stage failed to compile: {}", shaderName, type == GL_VERTEX_SHADER ? "vertex" : "fragment", logBuffer );
		glDeleteShader( stage );
		return 0;
	}

	return stage;
}
} // namespace

Shader::Shader( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

bool Shader::SplitStages( std::string_view source, std::string& outVertex, std::string& outFragment )
{
	outVertex.clear();
	outFragment.clear();

	// Note: Lines before the first "#type" line belong to no stage and are skipped
	std::string* currentStage = nullptr;

	std::istringstream stream{ std::string( source ) };
	std::string        line;
	while( std::getline( stream, line ) )
	{
		std::istringstream lineStream( line );
		std::string        keyword;
		lineStream >> keyword;

		if( keyword == "#type" )
		{
			std::string stageName;
			lineStream >> stageName;

			if( stageName == "vertex" )
			{
				currentStage = &outVertex;
			}
			else if( stageName == "fragment" )
			{
				currentStage = &outFragment;
			}
			else
			{
				GAIA_ERROR( "Shader: unknown stage {}", stageName );
				return false;
			}
			continue;
		}

		if( currentStage != nullptr )
		{
			currentStage->append( line );
			currentStage->push_back( '\n' );
		}
	}

	return !outVertex.empty() && !outFragment.empty();
}

bool Shader::LoadInternal()
{
	std::ifstream file{ std::string( GetPath() ) };
	if( !file.is_open() )
	{
		GAIA_ERROR( "Shader {}: failed to open {}", GetName(), GetPath() );
		return false;
	}

	std::stringstream fileText;
	fileText << file.rdbuf();

	std::string vertexSource;
	std::string fragmentSource;
	if( !SplitStages( fileText.str(), vertexSource, fragmentSource ) )
	{
		GAIA_ERROR( "Shader {}: needs a \"#type vertex\" and a \"#type fragment\" section", GetName() );
		return false;
	}

	const u32 vertexStage   = CompileStage( GL_VERTEX_SHADER, vertexSource, GetName() );
	const u32 fragmentStage = CompileStage( GL_FRAGMENT_SHADER, fragmentSource, GetName() );
	if( vertexStage == 0 || fragmentStage == 0 )
	{
		glDeleteShader( vertexStage );
		glDeleteShader( fragmentStage );
		return false;
	}

	m_Program = glCreateProgram();
	glAttachShader( m_Program, vertexStage );
	glAttachShader( m_Program, fragmentStage );
	glLinkProgram( m_Program );

	// Note: The linked program keeps what it needs, the stages can go
	glDeleteShader( vertexStage );
	glDeleteShader( fragmentStage );

	int linkResult = 0;
	glGetProgramiv( m_Program, GL_LINK_STATUS, &linkResult );
	if( linkResult == GL_FALSE )
	{
		char logBuffer[512] = {};
		glGetProgramInfoLog( m_Program, sizeof( logBuffer ), nullptr, logBuffer );
		GAIA_ERROR( "Shader {}: failed to link: {}", GetName(), logBuffer );
		glDeleteProgram( m_Program );
		m_Program = 0;
		return false;
	}

	return true;
}

bool Shader::UnloadInternal()
{
	glDeleteProgram( m_Program );
	m_Program = 0;
	return true;
}

void Shader::Use() const
{
	glUseProgram( m_Program );
}

void Shader::SetInt( const char* name, int value ) const
{
	glUniform1i( glGetUniformLocation( m_Program, name ), value );
}

void Shader::SetVec3( const char* name, const Vec3& value ) const
{
	glUniform3f( glGetUniformLocation( m_Program, name ), value.x, value.y, value.z );
}

void Shader::SetMatrix44( const char* name, const Matrix44& value ) const
{
	glUniformMatrix4fv( glGetUniformLocation( m_Program, name ), 1, GL_FALSE, &value.m00 );
}
