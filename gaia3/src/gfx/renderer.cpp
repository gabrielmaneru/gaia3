#include "gaia3_pch.h"
#include "renderer.h"

#include <glad/glad.h>

void Renderer::Init(ProcAddressFn procAddressLoader)
{
	if (procAddressLoader == nullptr)
	{
		std::cerr << "Renderer init failed: invalid proc address loader\n";
		return;
	}

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(procAddressLoader)))
	{
		std::cerr << "Renderer init failed: GLAD loader initialization failed\n";
		return;
	}

	if (!InitializeDummyShaders())
	{
		std::cerr << "Failed to create dummy shaders\n";
		return;
	}

	if (!InitializeDummyVertexBuffer())
	{
		std::cerr << "Failed to create dummy vertex buffer\n";
		return;
	}

	m_IsInitialized = true;
}

void Renderer::Update()
{
	if (!m_IsInitialized)
		return;

	// Clear buffer
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw dummy mesh
	glUseProgram(m_Program);
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

	m_IsInitialized = false;
}

bool Renderer::InitializeDummyShaders()
{
	constexpr const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		void main()
		{
			gl_Position = vec4(aPos, 1.0);
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
}
