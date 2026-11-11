#include "gaia3_pch.h"
#include "shader.h"

#include <core/app.h>

#include <glad/glad.h>

#include <algorithm>
#include <fstream>
#include <sstream>

RTTI_DEFINE_RESOURCE( Shader,
                      RTTI_RESOURCE_FOLDER( "shaders" ),
                      RTTI_RESOURCE_EXTENSIONS( ".glsl" ) )

RTTI_DEFINE_RESOURCE( ShaderInclude,
                      RTTI_RESOURCE_FOLDER( "shaders" ),
                      RTTI_RESOURCE_EXTENSIONS( ".glsli" ) )

namespace
{
bool ReadTextFile( std::string_view path, std::string& outText )
{
	std::ifstream file{ std::string( path ) };
	if( !file.is_open() )
		return false;

	std::stringstream text;
	text << file.rdbuf();
	outText = text.str();
	return true;
}

// Note: The name between the quotes of an #include line, or empty when the line isn't one
std::string ReadIncludeName( const std::string& line )
{
	std::istringstream lineStream( line );
	std::string        keyword;
	lineStream >> keyword;
	if( keyword != "#include" )
		return {};

	const u64 open  = line.find( '"' );
	const u64 close = open != std::string::npos ? line.find( '"', open + 1 ) : std::string::npos;
	if( close == std::string::npos )
		return {};

	return line.substr( open + 1, close - open - 1 );
}

bool PasteIncludes( std::string_view source, int sourceIndex, const Shader::IncludeReader& readInclude, std::string& outSource, std::vector<std::string>& outSourceNames )
{
	std::istringstream stream{ std::string( source ) };
	std::string        line;
	int                lineNumber = 0;
	while( std::getline( stream, line ) )
	{
		++lineNumber;

		std::istringstream lineStream( line );
		std::string        keyword;
		lineStream >> keyword;
		if( keyword != "#include" )
		{
			outSource.append( line );
			outSource.push_back( '\n' );
			continue;
		}

		const std::string includeName = ReadIncludeName( line );
		if( includeName.empty() )
		{
			GAIA_ERROR( "Shader: {} line {}: an include needs its name in quotes, like #include \"pbr.glsli\"", outSourceNames[sourceIndex], lineNumber );
			return false;
		}

		// Note: Already pasted, so its functions exist already. Keeps the line count with an empty line
		if( std::find( outSourceNames.begin(), outSourceNames.end(), includeName ) != outSourceNames.end() )
		{
			outSource.push_back( '\n' );
			continue;
		}

		std::string includeSource;
		if( !readInclude( includeName, includeSource ) )
		{
			GAIA_ERROR( "Shader: {} line {}: include {} is not available", outSourceNames[sourceIndex], lineNumber, includeName );
			return false;
		}

		outSourceNames.push_back( includeName );
		const int includeIndex = static_cast<int>( outSourceNames.size() ) - 1;

		outSource.append( "#line 1 " + std::to_string( includeIndex ) + "\n" );
		if( !PasteIncludes( includeSource, includeIndex, readInclude, outSource, outSourceNames ) )
			return false;

		// Note: Back in this file, on the line after the #include
		outSource.append( "#line " + std::to_string( lineNumber + 1 ) + " " + std::to_string( sourceIndex ) + "\n" );
	}

	return true;
}

// Note: Driver messages name files by index, like "1(12)", so the indexes are listed after them
std::string ListSourceNames( const std::vector<std::string>& sourceNames )
{
	std::string list;
	for( u64 i = 0; i < sourceNames.size(); ++i )
	{
		list += ( i == 0 ? "" : ", " ) + std::to_string( i ) + " is " + sourceNames[i];
	}
	return list;
}

u32 CompileStage( u32 type, const std::string& source, std::string_view shaderName, const std::vector<std::string>& sourceNames )
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
		GAIA_ERROR( "Shader {}: {} stage failed to compile ({}): {}", shaderName, type == GL_VERTEX_SHADER ? "vertex" : "fragment", ListSourceNames( sourceNames ), logBuffer );
		glDeleteShader( stage );
		return 0;
	}

	return stage;
}
} // namespace

ShaderInclude::ShaderInclude( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

bool ShaderInclude::LoadInternal()
{
	if( !ReadTextFile( GetPath(), m_Source ) )
	{
		GAIA_ERROR( "ShaderInclude {}: failed to open {}", GetName(), GetPath() );
		return false;
	}
	return true;
}

bool ShaderInclude::UnloadInternal()
{
	m_Source.clear();
	return true;
}

Shader::Shader( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

bool Shader::ResolveIncludes( std::string_view source, const IncludeReader& readInclude, std::string& outSource, std::vector<std::string>& outSourceNames )
{
	outSource.clear();
	outSourceNames.clear();
	outSourceNames.emplace_back( "main" );

	return PasteIncludes( source, 0, readInclude, outSource, outSourceNames );
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
	std::string fileText;
	if( !ReadTextFile( GetPath(), fileText ) )
	{
		GAIA_ERROR( "Shader {}: failed to open {}", GetName(), GetPath() );
		return false;
	}

	std::string vertexSource;
	std::string fragmentSource;
	if( !SplitStages( fileText, vertexSource, fragmentSource ) )
	{
		GAIA_ERROR( "Shader {}: needs a \"#type vertex\" and a \"#type fragment\" section", GetName() );
		return false;
	}

	// Note: Includes are .glsli resources, found by name like any resource, so an app can override them too
	ResourceManager& manager     = App::GetSession().GetResourceManager();
	auto             readInclude = [&manager]( std::string_view includeName, std::string& outSource ) {
		const shared<ShaderInclude> include = manager.FindResource<ShaderInclude>( includeName, true );
		if( include == nullptr )
			return false;

		outSource = include->GetSource();
		return true;
	};

	// Note: Each stage gets its own copy of what it includes
	std::string              vertexWithIncludes;
	std::string              fragmentWithIncludes;
	std::vector<std::string> vertexSourceNames;
	std::vector<std::string> fragmentSourceNames;
	if( !ResolveIncludes( vertexSource, readInclude, vertexWithIncludes, vertexSourceNames ) || !ResolveIncludes( fragmentSource, readInclude, fragmentWithIncludes, fragmentSourceNames ) )
	{
		GAIA_ERROR( "Shader {}: an include could not be pasted", GetName() );
		return false;
	}
	vertexSourceNames[0]   = GetName();
	fragmentSourceNames[0] = GetName();

	const u32 vertexStage   = CompileStage( GL_VERTEX_SHADER, vertexWithIncludes, GetName(), vertexSourceNames );
	const u32 fragmentStage = CompileStage( GL_FRAGMENT_SHADER, fragmentWithIncludes, GetName(), fragmentSourceNames );
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

void Shader::SetFloat( const char* name, float value ) const
{
	glUniform1f( glGetUniformLocation( m_Program, name ), value );
}

void Shader::SetVec3( const char* name, const Vec3& value ) const
{
	glUniform3f( glGetUniformLocation( m_Program, name ), value.x, value.y, value.z );
}

void Shader::SetMatrix44( const char* name, const Matrix44& value ) const
{
	glUniformMatrix4fv( glGetUniformLocation( m_Program, name ), 1, GL_FALSE, &value.m00 );
}
