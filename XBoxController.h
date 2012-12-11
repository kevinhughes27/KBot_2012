#ifndef XBOX_CONTROLLER_H
#define XBOX_CONTROLLER_H

#include "Joystick.h"

class XboxButton
{
public:
	XboxButton(int btnNumber, Joystick *joy);
	~XboxButton(){};
	
	void Refresh();
	
	bool Get();
	bool OnPress();
	bool OnRelease();
	
private:
	int buttonNumber;
	bool buttonState;
	bool lastButtonStatePress;
	bool lastButtonStateRelease;
	Joystick *joystick;
};

class XBoxController
{
public:
	XBoxController(unsigned port);
	virtual ~ XBoxController();
	
	void RefreshController();

	float GetLeftX(void);
	float GetLeftY(void);
	float GetRightX(void);
	float GetRightY(void);
	float GetTriggers(void);
	
	XboxButton *AButton;
	XboxButton *BButton;
	XboxButton *XButton;
	XboxButton *YButton;
	
	XboxButton *RightBumper;
	XboxButton *LeftBumper;
	
	XboxButton *BackButton;
	XboxButton *StartButton;
	
	XboxButton *RightPress;
	XboxButton *LeftPress;

private:
	Joystick *joystick;
	
	float LeftX;
	float LeftY;
	float RightX;
	float RightY;
	float Triggers;
};


#endif // XBOX_CONTROLLER_H
