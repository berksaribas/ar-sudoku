#include "input.h"
#include <cassert>
#include <iostream>

Input* Input::s_pInput = nullptr;

Input::Input(GLFWwindow& a_Window)
	: m_pGLFWWindowRef{ &a_Window }
{
	if (s_pInput == nullptr)
	{
		s_pInput = this;
		SetInputCallbacks(m_pGLFWWindowRef);
	}

	memset(m_arrCurrentKeyCode, KEY_UNTOUCHED, sizeof(m_arrCurrentKeyCode));
	memset(m_arrCurrentMouseCode, KEY_UNTOUCHED, sizeof(m_arrCurrentMouseCode));
}

Input::~Input()
{
	if (s_pInput == this)
	{
		m_pGLFWWindowRef = nullptr;
		s_pInput = nullptr;
	}
}

Input* Input::initialize(GLFWwindow& a_Window)
{
	if (s_pInput != nullptr)
	{
		std::cout << "Input::initialize:: Input already exists.\n";
		return nullptr;
	}
	s_pInput = new Input(a_Window);
	return s_pInput;
}

void Input::destroy()
{
	delete s_pInput;
	s_pInput = nullptr;
}

const Input* Input::get()
{
	return s_pInput;
}

void Input::SetInputCallbacks(GLFWwindow*& a_pGLFWwindow)
{
	glfwSetCursorPosCallback(a_pGLFWwindow, OnGetCursorPosition);
	glfwSetKeyCallback(a_pGLFWwindow, OnKeyboardBtnPressed);
	glfwSetMouseButtonCallback(a_pGLFWwindow, OnMouseBtnPressed);
	glfwSetScrollCallback(a_pGLFWwindow, OnMouseScroll);
}

void Input::OnKeyboardBtnPressed(GLFWwindow* a_pGLFWwindow, int a_iKey, int a_iScancode, int a_iAction, int a_iMods)
{
	KEY_STATE l_KeyState = (KEY_STATE)a_iAction;
	s_pInput->m_arrCurrentKeyCode[a_iKey] = l_KeyState;

	if (l_KeyState == KEY_PRESS || l_KeyState == KEY_RELEASED)
	{
		KeyEvent l_KeyEvent;
		l_KeyEvent.m_iKeyIndex = a_iKey;
		l_KeyEvent.m_KeyState = l_KeyState;

		s_pInput->m_StackKeyInput.push(l_KeyEvent);
	}
}

void Input::OnMouseBtnPressed(GLFWwindow* a_pFLFWwindow, int a_iButton, int a_iAction, int a_iMods)
{
	KEY_STATE l_KeyState = (KEY_STATE)a_iAction;
	s_pInput->m_arrCurrentMouseCode[a_iButton] = l_KeyState;

	if (l_KeyState == KEY_PRESS || l_KeyState == KEY_RELEASED)
	{
		KeyEvent l_KeyEvent;
		l_KeyEvent.m_iKeyIndex = a_iButton;
		l_KeyEvent.m_KeyState = l_KeyState;

		s_pInput->m_StackMouseInput.push(l_KeyEvent);
	}
}

void Input::OnMouseScroll(GLFWwindow* a_pWindow, double a_dXoffset, double a_dYoffset)
{
	s_pInput->m_iScrollValue = (int)(a_dYoffset);
}

void Input::OnGetCursorPosition(GLFWwindow* a_pGLFWwindow, double a_dPositionX, double a_dPositionY)
{
	s_pInput->m_dMousePositionX = a_dPositionX;
	s_pInput->m_dMousePositionY = a_dPositionY;
}

void Input::GetMousePosition(double& a_dMouseX, double& a_dMouseY)
{
	a_dMouseX = s_pInput->m_dMousePositionX;
	a_dMouseY = s_pInput->m_dMousePositionY;
}

bool Input::IsKeyDown(const int a_iKeyCode)
{
	assert(a_iKeyCode < MAX_KEYBOARD_BTNS && "KeyCode is more than max keyboard keys");
	return (s_pInput->m_arrCurrentKeyCode[a_iKeyCode] == KEY_PRESS || 
		s_pInput->m_arrCurrentKeyCode[a_iKeyCode] == KEY_REPEAT);
}

bool Input::IsKeyPutDown(const int a_iKeyCode)
{
	assert(a_iKeyCode < MAX_KEYBOARD_BTNS && "KeyCode is more than max keyboard keys");
	return s_pInput->m_arrCurrentKeyCode[a_iKeyCode] == KEY_PRESS;
}

bool Input::IsKeyPutUp(const int a_iKeyCode)
{
	assert(a_iKeyCode < MAX_KEYBOARD_BTNS && "KeyCode is more than max keyboard keys");
	return s_pInput->m_arrCurrentKeyCode[a_iKeyCode] == KEY_RELEASED;
}

bool Input::IsMouseBtnDown(const int a_iMouseKeyCode)
{
	assert(a_iMouseKeyCode < MAX_MOUSE_BTNS && "KeyCode is more than max mouse buttons");
	return (s_pInput->m_arrCurrentMouseCode[a_iMouseKeyCode] == KEY_PRESS ||
		s_pInput->m_arrCurrentMouseCode[a_iMouseKeyCode] == KEY_REPEAT);;
}

bool Input::IsMouseBtnPutDown(const int a_iMouseKeyCode)
{
	assert(a_iMouseKeyCode < MAX_MOUSE_BTNS && "KeyCode is more than max mouse buttons");
	return s_pInput->m_arrCurrentMouseCode[a_iMouseKeyCode] == KEY_PRESS;
}

bool Input::IsMouseBtnPutUp(const int a_iMouseKeyCode)
{
	assert(a_iMouseKeyCode < MAX_MOUSE_BTNS && "KeyCode is more than max mouse buttons");
	return s_pInput->m_arrCurrentMouseCode[a_iMouseKeyCode] == KEY_RELEASED;
}

void Input::setCursorEnability(bool a_bIsEnabled, bool a_bIsCursorVisible)
{
	glfwSetInputMode(s_pInput->m_pGLFWWindowRef, GLFW_CURSOR, a_bIsEnabled ? (a_bIsCursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN): GLFW_CURSOR_DISABLED);
}

int Input::GetMouseScroll()
{
	return s_pInput->m_iScrollValue;
}

void Input::Update()
{
	KeyEvent l_StackKeyEvent;
	while (!m_StackKeyInput.empty())
	{
		l_StackKeyEvent = m_StackKeyInput.top();
		m_arrCurrentKeyCode[l_StackKeyEvent.m_iKeyIndex] = (m_arrCurrentKeyCode[l_StackKeyEvent.m_iKeyIndex] == KEY_PRESS) ? KEY_REPEAT : KEY_UNTOUCHED;
		m_StackKeyInput.pop();
	}

	while (!m_StackMouseInput.empty())
	{
		l_StackKeyEvent = m_StackMouseInput.top();
		m_arrCurrentMouseCode[l_StackKeyEvent.m_iKeyIndex] = (m_arrCurrentMouseCode[l_StackKeyEvent.m_iKeyIndex] == KEY_PRESS) ? KEY_REPEAT : KEY_UNTOUCHED;
		m_StackMouseInput.pop();
	}

	m_iScrollValue = 0;
}