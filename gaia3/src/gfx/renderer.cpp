#include "gaia3_pch.h"
#include "renderer.h"

#include <glad/glad.h>

bool Renderer::Init(ProcAddressFn procAddressLoader, rect canvas)
{
	assert(procAddressLoader != nullptr);
	assert(canvas.x > 0 && canvas.y > 0);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(procAddressLoader)))
	{
		std::cerr << "Renderer init failed: GLAD loader initialization failed\n";
		return false;
	}

	if (!InitializeDummyShaders())
	{
		std::cerr << "Renderer init failed: Failed to create dummy shaders\n";
		return false;
	}

	if (!InitializeDummyVertexBuffer())
	{
		std::cerr << "Renderer init failed: Failed to create dummy vertex buffer\n";
		return false;
	}

	m_Camera.Init(canvas);

	return true;
}

void Renderer::Update(rect canvas)
{
	glViewport(0, 0, canvas.x, canvas.y);

	// Clear buffer
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw dummy mesh
	glUseProgram(m_Program);

	m_Camera.Update(canvas);
	const Matrix44& view = m_Camera.GetViewMatrix();
	const Matrix44& projection = m_Camera.GetProjectionMatrix();
	glUniformMatrix4fv(m_ViewMatrixLocation, 1, GL_FALSE, &view.col_0.x);
	glUniformMatrix4fv(m_ProjectionMatrixLocation, 1, GL_FALSE, &projection.col_0.x);
	glBindVertexArray(m_VertexArray);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void Renderer::Shutdown()
{
	if (m_VertexBuffer != 0)
	{
		glDeleteBuffers(1, &m_VertexBuffer);
		m_VertexBuffer = 0;
	}

	if (m_VertexArray != 0)
	{
		glDeleteVertexArrays(1, &m_VertexArray);
		m_VertexArray = 0;
	}

	if (m_Program != 0)
	{
		glDeleteProgram(m_Program);
		m_Program = 0;
	}
}

bool Renderer::InitializeDummyShaders()
{
	constexpr const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		uniform mat4 u_View;
		uniform mat4 u_Projection;
		void main()
		{
			gl_Position = u_Projection * u_View * vec4(aPos, 1.0);
		}
	)";

	constexpr const char* fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		void main()
		{
			FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
	)";

	auto lambdaCompileShader = [](u32 type, const char* source) -> u32
	{
		const u32 shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		int compileResult = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
		if (compileResult == GL_FALSE)
		{
			char logBuffer[512] = {};
			glGetShaderInfoLog(shader, sizeof(logBuffer), nullptr, logBuffer);
			std::cerr << "Shader compilation failed: " << logBuffer << '\n';
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	};

	// Compile Vtx & Frag shaders
	const u32 vertexShader = lambdaCompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	const u32 fragmentShader = lambdaCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	if (vertexShader == 0 || fragmentShader == 0)
	{
		if (vertexShader != 0)
			glDeleteShader(vertexShader);
		if (fragmentShader != 0)
			glDeleteShader(fragmentShader);
		return false;
	}

	// Create Program and link shaders
	m_Program = glCreateProgram();
	glAttachShader(m_Program, vertexShader);
	glAttachShader(m_Program, fragmentShader);
	glLinkProgram(m_Program);

	// Verify linking results
	int linkResult = 0;
	glGetProgramiv(m_Program, GL_LINK_STATUS, &linkResult);
	if (linkResult == GL_FALSE)
	{
		char logBuffer[512] = {};
		glGetProgramInfoLog(m_Program, sizeof(logBuffer), nullptr, logBuffer);
		std::cerr << "Shader link failed: " << logBuffer << '\n';
		glDeleteProgram(m_Program);
		m_Program = 0;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}

	// Delete shader cache
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_ViewMatrixLocation = glGetUniformLocation(m_Program, "u_View");
	m_ProjectionMatrixLocation = glGetUniformLocation(m_Program, "u_Projection");
	if (m_ViewMatrixLocation < 0 || m_ProjectionMatrixLocation < 0)
	{
		std::cerr << "Renderer init failed: Camera matrix uniform lookup failed\n";
		return false;
	}

	return true;
}

bool Renderer::InitializeDummyVertexBuffer()
{
	constexpr float triangleVertices[] = {
		0.0f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};

	glGenVertexArrays(1, &m_VertexArray);
	glGenBuffers(1, &m_VertexBuffer);

	glBindVertexArray(m_VertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}
