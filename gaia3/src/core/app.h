#pragma once

#include <core/session.h>
#include <editor/editorShell.h>
#include <gfx/renderer.h>
#include <platform/input.h>
#include <platform/window.h>

#include <functional>
#include <string>

struct AppConfig
{
	rect        canvas = { 800, 600 };
	std::string title  = "";

	std::string startLevel = "";

	std::string appPath    = "";
	std::string commonPath = R"(..\..\gaia3\assets\)";

	EditorConfig editor;

	// Note: Builds the editor when editor.enabled is true, so a sample can use its own editor. Empty builds the
	// LevelEditor
	std::function<unique<EditorShell>( const EditorConfig& )> createEditor;

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

	// Note: Null without an editor
	static EditorShell* GetEditor() { return m_Editor.get(); }

private:
	static Window              m_Window;
	static Input               m_Input;
	static Renderer            m_Renderer;
	static Session             m_Session;
	static unique<EditorShell> m_Editor;
	static u64            m_MaxFrames;
};
