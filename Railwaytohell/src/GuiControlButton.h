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

	void SetTextures(SDL_Texture* normal, SDL_Texture* focused, SDL_Texture* pressed, SDL_Texture* off);
	void SetState(GuiControlState newState) {
		state = newState;
	}
private:
	GuiControlState prevState; // Store the previous state of the button

	SDL_Texture* normalTex = nullptr;
	SDL_Texture* focusedTex = nullptr;
	SDL_Texture* pressedTex = nullptr;
	SDL_Texture* offTex = nullptr;

	int OnfocussedFx, OnPressedFx, OnReleasedFx; // Sound effects for button states
	bool canClick = true;
	bool drawBasic = false;
};

#pragma once