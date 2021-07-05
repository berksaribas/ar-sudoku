#pragma once

enum BTN_TYPE
{
	BTN_CONFIRM = 0,
	BTN_SOLUTION = 1,
	BTN_HELP = 2,
	BTN_COUNT = 3,
};

enum BTN_CONDITION
{
	BTN_NORMAL = 0,
	BTN_HOVER = 1,
	BTN_PRESSED = 2,
	BTN_HIDDEN = 3
};

class UIButton : public SquareData
{
public:

	UIButton(BTN_TYPE a_BtnType, BTN_CONDITION a_BtnCondition, SquareData&& a_SquareData);

	BTN_TYPE m_BtnType = BTN_CONFIRM;
	BTN_CONDITION m_BtnCondition = BTN_NORMAL;

	// Is the button visible
	bool isBtnShown();

	// Shows/ hides the button
	void visibilityToggle(bool a_bIsVisible);

	///Returns true if the button was selected this frame
	bool manageButton();
};