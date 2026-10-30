#pragma once

enum class eInputKeys : u16
{
	// Copy from GLFW_KEY defines
	/* Printable keys */
	SPACE            = 32,
	APOSTROPHE       = 39,  /* ' */
	COMMA            = 44,  /* , */
	MINUS            = 45,  /* - */
	PERIOD           = 46,  /* . */
	SLASH            = 47,  /* / */
	NUM_0            = 48,
	NUM_1            = 49,
	NUM_2            = 50,
	NUM_3            = 51,
	NUM_4            = 52,
	NUM_5            = 53,
	NUM_6            = 54,
	NUM_7            = 55,
	NUM_8            = 56,
	NUM_9            = 57,
	SEMICOLON        = 59,  /* ; */
	EQUAL            = 61,  /* = */
	A                = 65,
	B                = 66,
	C                = 67,
	D                = 68,
	E                = 69,
	F                = 70,
	G                = 71,
	H                = 72,
	I                = 73,
	J                = 74,
	K                = 75,
	L                = 76,
	M                = 77,
	N                = 78,
	O                = 79,
	P                = 80,
	Q                = 81,
	R                = 82,
	S                = 83,
	T                = 84,
	U                = 85,
	V                = 86,
	W                = 87,
	X                = 88,
	Y                = 89,
	Z                = 90,
	LEFT_BRACKET     = 91,  /* [ */
	BACKSLASH        = 92,  /* \ */
	RIGHT_BRACKET    = 93,  /* ] */
	GRAVE_ACCENT     = 96,  /* ` */
	WORLD_1          = 161, /* non-US #1 */
	WORLD_2          = 162, /* non-US #2 */
	/* Function keys */
	ESCAPE           = 256,
	ENTER            = 257,
	TAB              = 258,
	BACKSPACE        = 259,
	INSERT           = 260,
	DELETE           = 261,
	RIGHT            = 262,
	LEFT             = 263,
	DOWN             = 264,
	UP               = 265,
	PAGE_UP          = 266,
	PAGE_DOWN        = 267,
	HOME             = 268,
	END              = 269,
	CAPS_LOCK        = 280,
	SCROLL_LOCK      = 281,
	NUM_LOCK         = 282,
	PRINT_SCREEN     = 283,
	PAUSE            = 284,
	F1               = 290,
	F2               = 291,
	F3               = 292,
	F4               = 293,
	F5               = 294,
	F6               = 295,
	F7               = 296,
	F8               = 297,
	F9               = 298,
	F10              = 299,
	F11              = 300,
	F12              = 301,
	F13              = 302,
	F14              = 303,
	F15              = 304,
	F16              = 305,
	F17              = 306,
	F18              = 307,
	F19              = 308,
	F20              = 309,
	F21              = 310,
	F22              = 311,
	F23              = 312,
	F24              = 313,
	F25              = 314,
	KP_0             = 320,
	KP_1             = 321,
	KP_2             = 322,
	KP_3             = 323,
	KP_4             = 324,
	KP_5             = 325,
	KP_6             = 326,
	KP_7             = 327,
	KP_8             = 328,
	KP_9             = 329,
	KP_DECIMAL       = 330,
	KP_DIVIDE        = 331,
	KP_MULTIPLY      = 332,
	KP_SUBTRACT      = 333,
	KP_ADD           = 334,
	KP_ENTER         = 335,
	KP_EQUAL         = 336,
	LEFT_SHIFT       = 340,
	LEFT_CONTROL     = 341,
	LEFT_ALT         = 342,
	LEFT_SUPER       = 343,
	RIGHT_SHIFT      = 344,
	RIGHT_CONTROL    = 345,
	RIGHT_ALT        = 346,
	RIGHT_SUPER      = 347,
	MENU             = 348,
	_KEY_COUNT
};

enum class eInputMouseButtons : u16
{
	// Copy from GLFW_MOUSE_BUTTON defines
	NUM_1         = 0,
	NUM_2         = 1,
	NUM_3         = 2,
	NUM_4         = 3,
	NUM_5         = 4,
	NUM_6         = 5,
	NUM_7         = 6,
	NUM_8         = 7,
	_BUTTON_COUNT,

	LEFT          = NUM_1,
	RIGHT         = NUM_2,
	MIDDLE        = NUM_3,
};

enum class eInputState : u8
{
	UP = 0,
	DOWN,
	TRIGGERED,
	RELEASED,
};

struct GLFWwindow;

class Input
{
public:
	bool Init(GLFWwindow* windowHandle);
	void Update();

	bool IsKeyDown(eInputKeys inputKey) const;
	bool IsKeyUp(eInputKeys inputKey) const;
	bool IsKeyTriggered(eInputKeys inputKey) const;
	bool IsKeyReleased(eInputKeys inputKey) const;

	bool IsMouseButtonDown(eInputMouseButtons inputButton) const;
	bool IsMouseButtonUp(eInputMouseButtons inputButton) const;
	bool IsMouseButtonTriggered(eInputMouseButtons inputButton) const;
	bool IsMouseButtonReleased(eInputMouseButtons inputButton) const;

	double GetMouseX() const { return m_MouseX; }
	double GetMouseY() const { return m_MouseY; }
	double GetMouseDeltaX() const { return m_MouseDeltaX; }
	double GetMouseDeltaY() const { return m_MouseDeltaY; }

	static Input* _GetInstance() { assert(s_Instance != nullptr); return s_Instance; }

private:
	static void KeyCallback(GLFWwindow* windowHandle, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* windowHandle, int button, int action, int mods);
	static void MousePositionCallback(GLFWwindow* windowHandle, double xPos, double yPos);

	static Input* s_Instance;

	eInputState m_KeyStates[(u16)eInputKeys::_KEY_COUNT] = {};
	eInputState m_MouseButtonStates[(u16)eInputMouseButtons::_BUTTON_COUNT] = {};
	
	double m_MouseX = 0.0;
	double m_MouseY = 0.0;
	double m_PrevMouseX = 0.0;
	double m_PrevMouseY = 0.0;
	double m_MouseDeltaX = 0.0;
	double m_MouseDeltaY = 0.0;
	bool m_HasMousePosition = false;
};
#define input Input::_GetInstance()
