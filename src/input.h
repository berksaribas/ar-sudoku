#pragma once
#include <GLFW/glfw3.h>
#include <stack>

class  Input
{
private:

	enum KEY_STATE
	{
		KEY_RELEASED = 0,
		KEY_PRESS = 1,
		KEY_REPEAT = 2,
		KEY_UNTOUCHED = 3
	};

	struct KeyEvent
	{
		KEY_STATE m_KeyState = KEY_UNTOUCHED;
		int m_iKeyIndex = 0;
	};

	//singleton instance
	static Input* s_pInput;

	GLFWwindow* m_pGLFWWindowRef = nullptr;

	static constexpr int MAX_KEYBOARD_BTNS = 1024;
	static constexpr int MAX_MOUSE_BTNS = 32;

	KEY_STATE m_arrCurrentKeyCode[MAX_KEYBOARD_BTNS] = {};
	KEY_STATE m_arrCurrentMouseCode[MAX_MOUSE_BTNS] = {};

	double m_dMousePositionX = 0.0f;
	double m_dMousePositionY = 0.0f;

	int m_iScrollValue = 0;

	//Sets the GLFW callbacks for key, mouse and pinter inputs.
	void SetInputCallbacks(GLFWwindow*& a_pGLFWwindow);

	//GLFW callback called on keyboard btn down
	static void OnKeyboardBtnPressed(GLFWwindow* a_pGLFWwindow, int a_iKey, int a_iScancode, int a_iAction, int a_iMods);

	//GLFW callback called on mouse btn down
	static void OnMouseBtnPressed(GLFWwindow* a_pFLFWwindow, int a_iButton, int a_iAction, int a_iMods);

	//GLFW gets the current mouse position
	static void OnGetCursorPosition(GLFWwindow* a_pGLFWwindow, double a_dPositionX, double a_dPositionY);

	//GLFW event called on mouse scroll.
	static void OnMouseScroll(GLFWwindow* a_pWindow, double a_dXoffset, double a_dYoffset);

	std::stack<KeyEvent> m_StackKeyInput;
	std::stack<KeyEvent> m_StackMouseInput;

	Input(GLFWwindow& a_Window);
	~Input();

public:
	static Input* initialize(GLFWwindow& a_Window);
	void destroy();
	static const Input* get();

	void Update();

	//Returns mouse position
	static void GetMousePosition(double& a_dMouseX, double& a_dMouseY);

	//Returns the current frame mouse scroll value.
	// If value is -1 mouse is scrolled backwards else if +1 its scrolled forward.
	static int GetMouseScroll();

	//Checks if keyboard key with given keycode is down
	static bool IsKeyDown(const int a_iKeyCode);

	//Checks if keyboard key with given keycode is put down
	static bool IsKeyPutDown(const int a_iKeyCode);

	//Checks if keyboard key with given keycode is put up
	static bool IsKeyPutUp(const int a_iKeyCode);

	//Checks if mouse key with given keycode is down
	static bool IsMouseBtnDown(const int a_iMouseKeyCode);

	//Checks if mouse key with given keycode is put down
	static bool IsMouseBtnPutDown(const int a_iMouseKeyCode);

	//Checks if mouse key with given keycode is put up
	static bool IsMouseBtnPutUp(const int a_iKeyCode);

	//Sets the cursor to enabled or disabled. If disabled the cursor will have no window bounds and will not be visible on screen.
	//If enabled the second parameter will set if the cursor is visible or not.
	static void setCursorEnability(bool a_bIsEnabled, bool a_bIsCursorVisible = true);
};