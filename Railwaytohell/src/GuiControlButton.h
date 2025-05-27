#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	bool Start() override;
	// Called each loop iteration
	bool Update(float dt);

private:
	GuiControlState prevState; // Store the previous state of the button

	int OnfocussedFx, OnPressedFx, OnReleasedFx; // Sound effects for button states
	bool canClick = true;
	bool drawBasic = false;
};

#pragma once