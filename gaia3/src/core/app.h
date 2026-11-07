#pragma once

#include <gfx/renderer.h>
#include <core/session.h>
#include <platform/input.h>
#include <platform/window.h>
#include <scene/editor.h>

#include <string>

struct AppConfig
{
	rect        canvas = { 800, 600 };
	std::string title  = "";

	std::string startLevel = "";

	std::string appPath    = "";
	std::string commonPath = R"(..\..\gaia3\assets\)";

	EditorConfig editor;

	// Note: Zero runs until the window closes
	u64 maxFrames = 0;
};

class App
{
public:
	static bool Init( const AppConfig& config );
	static void Run();
	static void Shutdown();

	// External accesors
	static Window&   GetWindow() { return m_Window; }
	static Renderer& GetRenderer() { return m_Renderer; }
	static Session&  GetSession() { return m_Session; }

private:
	static Window         m_Window;
	static Input          m_Input;
	static Renderer       m_Renderer;
	static Session        m_Session;
	static unique<Editor> m_Editor;
	static u64            m_MaxFrames;
};