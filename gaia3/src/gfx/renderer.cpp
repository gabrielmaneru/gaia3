#include "gaia3_pch.h"
#include "renderer.h"

#include <glad/glad.h>

#include <algorithm>

bool Renderer::Init( ProcAddressFn procAddressLoader, rect canvas )
{
	assert( procAddressLoader != nullptr );
	assert( canvas.x > 0 && canvas.y > 0 );

	if( !gladLoadGLLoader( reinterpret_cast<GLADloadproc>( procAddressLoader ) ) )
	{
		std::cerr << "Renderer init failed: GLAD loader initialization failed\n";
		return false;
	}

	if( !InitializeDummyShaders() )
	{
		std::cerr << "Renderer init failed: Failed to create dummy shaders\n";
		return false;
	}

	glEnable( GL_DEPTH_TEST );

	return true;
}

void Renderer::Update( rect canvas )
{
	glViewport( 0, 0, canvas.x, canvas.y );

	// Clear buffer
	glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( m_Program );

	// Apply Camera Matrices
	if( !m_Cameras.empty() )
	{
		// Use first camera for now
		const Matrix44 view       = m_Cameras[0]->GetViewMatrix();
		const Matrix44 projection = m_Cameras[0]->GetProjectionMatrix( canvas );
		glUniformMatrix4fv( m_ViewMatrixLocation, 1, GL_FALSE, &view.m00 );
		glUniformMatrix4fv( m_ProjectionMatrixLocation, 1, GL_FALSE, &projection.m00 );
	}
	else
	{
		glUniformMatrix4fv( m_ViewMatrixLocation, 1, GL_FALSE, &Matrix44::V_Identity.m00 );
		glUniformMatrix4fv( m_ProjectionMatrixLocation, 1, GL_FALSE, &Matrix44::V_Identity.m00 );
	}

	for( const Renderable* renderable : m_Renderables )
	{
		const Matrix44 model = renderable->GetModelMatrix();
		glUniformMatrix4fv( m_ModelMatrixLocation, 1, GL_FALSE, &model.m00 );

		renderable->Draw();
	}
}

void Renderer::Shutdown()
{
	m_Renderables.clear();
	m_Cameras.clear();

	if( m_Program != 0 )
	{
		glDeleteProgram( m_Program );
		m_Program = 0;
	}
}

bool Renderer::InitializeDummyShaders()
{
	constexpr const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 aNormal;
		uniform mat4 u_Model;
		uniform mat4 u_View;
		uniform mat4 u_Projection;
		out vec3 vNormal;
		void main()
		{
			vNormal = aNormal;
			gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
		}
	)";

	// Note: Faces are colored by normal so the shape is readable without lighting
	constexpr const char* fragmentShaderSource = R"(
		#version 330 core
		in vec3 vNormal;
		out vec4 FragColor;
		void main()
		{
			FragColor = vec4(vNormal * 0.5 + 0.5, 1.0);
		}
	)";

	auto lambdaCompileShader = []( u32 type, const char* source ) -> u32 {
		const u32 shader = glCreateShader( type );
		glShaderSource( shader, 1, &source, nullptr );
		glCompileShader( shader );

		int compileResult = 0;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &compileResult );
		if( compileResult == GL_FALSE )
		{
			char logBuffer[512] = {};
			glGetShaderInfoLog( shader, sizeof( logBuffer ), nullptr, logBuffer );
			std::cerr << "Shader compilation failed: " << logBuffer << '\n';
			glDeleteShader( shader );
			return 0;
		}

		return shader;
	};

	// Compile Vtx & Frag shaders
	const u32 vertexShader   = lambdaCompileShader( GL_VERTEX_SHADER, vertexShaderSource );
	const u32 fragmentShader = lambdaCompileShader( GL_FRAGMENT_SHADER, fragmentShaderSource );
	if( vertexShader == 0 || fragmentShader == 0 )
	{
		if( vertexShader != 0 )
			glDeleteShader( vertexShader );
		if( fragmentShader != 0 )
			glDeleteShader( fragmentShader );
		return false;
	}

	// Create Program and link shaders
	m_Program = glCreateProgram();
	glAttachShader( m_Program, vertexShader );
	glAttachShader( m_Program, fragmentShader );
	glLinkProgram( m_Program );

	// Verify linking results
	int linkResult = 0;
	glGetProgramiv( m_Program, GL_LINK_STATUS, &linkResult );
	if( linkResult == GL_FALSE )
	{
		char logBuffer[512] = {};
		glGetProgramInfoLog( m_Program, sizeof( logBuffer ), nullptr, logBuffer );
		std::cerr << "Shader link failed: " << logBuffer << '\n';
		glDeleteProgram( m_Program );
		m_Program = 0;
		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );
		return false;
	}

	// Delete shader cache
	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );

	m_ModelMatrixLocation      = glGetUniformLocation( m_Program, "u_Model" );
	m_ViewMatrixLocation       = glGetUniformLocation( m_Program, "u_View" );
	m_ProjectionMatrixLocation = glGetUniformLocation( m_Program, "u_Projection" );
	if( m_ModelMatrixLocation < 0 || m_ViewMatrixLocation < 0 || m_ProjectionMatrixLocation < 0 )
	{
		std::cerr << "Renderer init failed: Matrix uniform lookup failed\n";
		return false;
	}

	return true;
}
