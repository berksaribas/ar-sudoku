#include "Renderer.h"
#include "UIButton.h"
#include "input.h"

UIButton::UIButton(BTN_TYPE a_BtnType, BTN_CONDITION a_BtnCondition, SquareData&& a_SquareData)
	: SquareData( a_SquareData ), m_BtnType{ a_BtnType }, m_BtnCondition{ a_BtnCondition }
{
	
}

// Is the button visible
bool UIButton::isBtnShown()
{ 
	return is_visible;
};

void UIButton::visibilityToggle(bool a_bIsVisible)
{
	m_BtnCondition = a_bIsVisible ? BTN_NORMAL : BTN_HIDDEN;
}

bool UIButton::manageButton()
{
	bool l_bIsBtnSelected = false;
	if(m_BtnCondition == BTN_HIDDEN)
	{
		is_visible = false;
		return l_bIsBtnSelected;
	}

	is_visible = true;

	double l_dMouseXPos = 0.0f;
	double l_dMouseYPos = 0.0f;
	Input::GetMousePosition(l_dMouseXPos, l_dMouseYPos);

	float l_fWidthDiv2 = width * 0.5f;
	float l_fHeightDiv2 = height * 0.5f;
	// If mouse pointer is over the button return
	if (l_dMouseXPos >= (x - l_fWidthDiv2) && l_dMouseXPos <= (x + l_fWidthDiv2) &&
		l_dMouseYPos >= (y - l_fHeightDiv2) && l_dMouseYPos <= (y + l_fHeightDiv2))
	{
		if (m_BtnCondition == BTN_CONDITION::BTN_NORMAL)
		{
			m_BtnCondition = BTN_CONDITION::BTN_HOVER;
		}
		else if (m_BtnCondition == BTN_CONDITION::BTN_HOVER && Input::IsMouseBtnDown(GLFW_MOUSE_BUTTON_LEFT))
		{
			m_BtnCondition = BTN_CONDITION::BTN_PRESSED;
		}
		else if (m_BtnCondition == BTN_CONDITION::BTN_PRESSED && Input::IsMouseBtnPutUp(GLFW_MOUSE_BUTTON_LEFT))
		{
			l_bIsBtnSelected = true;
			m_BtnCondition = BTN_CONDITION::BTN_NORMAL;
		}
	}
	else
	{
		m_BtnCondition = BTN_CONDITION::BTN_NORMAL;
	}

	return l_bIsBtnSelected;
}