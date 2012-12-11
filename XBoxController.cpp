#include "XBoxController.h"

XboxButton::XboxButton(int btnNumber, Joystick *joy)
{
	buttonNumber = btnNumber;
	joystick = joy;
}
void XboxButton::Refresh()
{
	buttonState = joystick->GetRawButton(buttonNumber);
}

bool XboxButton::Get()
{
	return buttonState;
}

bool XboxButton::OnPress()
{
	bool isTrue = buttonState && !lastButtonStatePress;
	lastButtonStatePress = buttonState;
	return isTrue;
}

bool XboxButton::OnRelease()
{
	bool isTrue = !buttonState && lastButtonStateRelease;
	lastButtonStateRelease = buttonState;
	return isTrue;
}


XBoxController::XBoxController(unsigned port)
{	
	joystick = new Joystick(port);
	
	AButton = new XboxButton(1, joystick);
	BButton = new XboxButton(2, joystick);
	XButton = new XboxButton(3, joystick);
	YButton = new XboxButton(4, joystick);

	LeftBumper = new XboxButton(5, joystick);
	RightBumper = new XboxButton(6, joystick);

	StartButton = new XboxButton(7, joystick);
	BackButton = new XboxButton(8, joystick);
	
	LeftPress = new XboxButton(9, joystick);
	RightPress = new XboxButton(10, joystick);
	
	LeftX = 0;
	LeftY = 0;
	RightX = 0;
	RightY = 0;
	Triggers = 0;
}

XBoxController::~XBoxController() {}

//Axys
void XBoxController::RefreshController()
{
	LeftX = joystick->GetRawAxis(1);
	LeftY = joystick->GetRawAxis(2);
	RightX = joystick->GetRawAxis(4);
	RightY = joystick->GetRawAxis(5);
	Triggers = joystick->GetRawAxis(3);
	
	AButton->Refresh();
	BButton->Refresh();
	XButton->Refresh();
	YButton->Refresh();

	LeftBumper->Refresh();
	RightBumper->Refresh();

	BackButton->Refresh();
	StartButton->Refresh();
	
	LeftPress->Refresh();
	RightPress->Refresh();
}

float XBoxController::GetLeftX()
{
	return LeftX;
}

float XBoxController::GetLeftY()
{
	return LeftY;
}

float XBoxController::GetRightX()
{
	return RightX;
}

float XBoxController::GetRightY()
{
	return RightY;
}

float XBoxController::GetTriggers()
{
	return Triggers;
}

