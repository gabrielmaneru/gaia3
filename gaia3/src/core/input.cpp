#include "gaia3_pch.h"
#include "input.h"

#include <GLFW/glfw3.h>

Input* Input::s_Instance = nullptr;

bool Input::Init(GLFWwindow* windowHandle)
{
	assert(windowHandle != nullptr);
	s_Instance = this;

	glfwSetKeyCallback(windowHandle, &Input::KeyCallback);
	glfwSetCursorPosCallback(windowHandle, &Input::MousePositionCallback);
	glfwSetMouseButtonCallback(windowHandle, &Input::MouseButtonCallback);

	return true;
}

void Input::Update()
{
	for (u16 i = 0; i < (u16)eInputKeys::_KEY_COUNT; i++)
	{
		if (m_KeyStates[i] == eInputState::TRIGGERED)
			m_KeyStates[i] = eInputState::DOWN;
		else if (m_KeyStates[i] == eInputState::RELEASED)
			m_KeyStates[i] = eInputState::UP;
	}

	for (u16 i = 0; i < (u16)eInputMouseButtons::_BUTTON_COUNT; i++)
	{
		if (m_MouseButtonStates[i] == eInputState::TRIGGERED)
			m_MouseButtonStates[i] = eInputState::DOWN;
		else if (m_MouseButtonStates[i] == eInputState::RELEASED)
			m_MouseButtonStates[i] = eInputState::UP;
	}
}



bool Input::IsKeyDown(eInputKeys inputKey) const
{
	assert((u16)inputKey < (u16)eInputKeys::_KEY_COUNT);
	const eInputState state = m_KeyStates[(u16)inputKey];
	return state == eInputState::DOWN || state == eInputState::TRIGGERED;
}

bool Input::IsKeyUp(eInputKeys inputKey) const
{
	assert((u16)inputKey < (u16)eInputKeys::_KEY_COUNT);
	const eInputState state = m_KeyStates[(u16)inputKey];
	return state == eInputState::UP || state == eInputState::RELEASED;
}

bool Input::IsKeyTriggered(eInputKeys inputKey) const
{
	assert((u16)inputKey < (u16)eInputKeys::_KEY_COUNT);
	return m_KeyStates[(u16)inputKey] == eInputState::TRIGGERED;
}

bool Input::IsKeyReleased(eInputKeys inputKey) const
{
	assert((u16)inputKey < (u16)eInputKeys::_KEY_COUNT);
	return m_KeyStates[(u16)inputKey] == eInputState::RELEASED;
}

bool Input::IsMouseButtonDown(eInputMouseButtons inputButton) const
{
	assert((u16)inputButton < (u16)eInputMouseButtons::_BUTTON_COUNT);
	const eInputState state = m_MouseButtonStates[(u16)inputButton];
	return state == eInputState::DOWN || state == eInputState::TRIGGERED;
}

bool Input::IsMouseButtonUp(eInputMouseButtons inputButton) const
{
	assert((u16)inputButton < (u16)eInputMouseButtons::_BUTTON_COUNT);
	const eInputState state = m_MouseButtonStates[(u16)inputButton];
	return state == eInputState::UP || state == eInputState::RELEASED;
}

bool Input::IsMouseButtonTriggered(eInputMouseButtons inputButton) const
{
	assert((u16)inputButton < (u16)eInputMouseButtons::_BUTTON_COUNT);
	return m_MouseButtonStates[(u16)inputButton] == eInputState::TRIGGERED;
}

bool Input::IsMouseButtonReleased(eInputMouseButtons inputButton) const
{
	assert((u16)inputButton < (u16)eInputMouseButtons::_BUTTON_COUNT);
	return m_MouseButtonStates[(u16)inputButton] == eInputState::RELEASED;
}



void Input::KeyCallback(GLFWwindow* windowHandle, int key, int, int action, int)
{
	if (windowHandle == nullptr || s_Instance == nullptr)
		return;

	assert((u16)key < (u16)eInputKeys::_KEY_COUNT);

	if (action == GLFW_PRESS)
		s_Instance->m_KeyStates[key] = eInputState::TRIGGERED;
	else if (action == GLFW_RELEASE)
		s_Instance->m_KeyStates[key] = eInputState::RELEASED;
}

void Input::MouseButtonCallback(GLFWwindow* windowHandle, int button, int action, int)
{
	if (windowHandle == nullptr || s_Instance == nullptr)
		return;

	assert((u16)button < (u16)eInputMouseButtons::_BUTTON_COUNT);

	if (action == GLFW_PRESS)
		s_Instance->m_MouseButtonStates[button] = eInputState::TRIGGERED;
	else if (action == GLFW_RELEASE)
		s_Instance->m_MouseButtonStates[button] = eInputState::RELEASED;
}

void Input::MousePositionCallback(GLFWwindow* windowHandle, double xPos, double yPos)
{
	if (windowHandle == nullptr || s_Instance == nullptr)
		return;

	if (s_Instance->m_HasMousePosition == false)
	{
		s_Instance->m_PrevMouseX = xPos;
		s_Instance->m_PrevMouseY = yPos;
		s_Instance->m_HasMousePosition = true;
	}

	s_Instance->m_MouseX = xPos;
	s_Instance->m_MouseY = yPos;
	s_Instance->m_MouseDeltaX = xPos - s_Instance->m_PrevMouseX;
	s_Instance->m_MouseDeltaY = yPos - s_Instance->m_PrevMouseY;
	s_Instance->m_PrevMouseX = xPos;
	s_Instance->m_PrevMouseY = yPos;
}
