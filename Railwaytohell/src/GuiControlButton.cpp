#include "GuiControlButton.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Scene.h"

GuiControlButton::GuiControlButton(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;
	this->text = text;

	canClick = true;
	drawBasic = false;

	prevState = GuiControlState::NORMAL;

}

GuiControlButton::~GuiControlButton()
{

}

void GuiControlButton::SetTextures(SDL_Texture* normal, SDL_Texture* focused, SDL_Texture* pressed, SDL_Texture* off)
{
	normalTex = normal;
	focusedTex = focused;
	pressedTex = pressed;
	offTex = off;
}

bool GuiControlButton::Start()
{
	// Load the button sound effects
	OnfocussedFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/button-focussed.wav");
	OnPressedFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/button-pressed.wav");
	OnReleasedFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/button-focussed.wav"); 
	return true;
}

bool GuiControlButton::Update(float dt)
{
	if (Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY)
	{
		return true;
	}
	if (state == GuiControlState::ON || state == GuiControlState::OFF)
	{
		return false;
	}

	if (state != GuiControlState::DISABLED && state != GuiControlState::OFF )
	{
		// Check if the state has changed
		if (state != prevState)
		{
			// Then play the corresponding sound effect
			switch (state)
			{
			case GuiControlState::FOCUSED:
				Engine::GetInstance().audio->PlayFx(OnfocussedFx);
				break;
			case GuiControlState::PRESSED:
				Engine::GetInstance().audio->PlayFx(OnPressedFx);
				break;
			}
			prevState = state;
		}

		//Update the state of the GUiButton according to the mouse position
		Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

		//If the position of the mouse if inside the bounds of the button 
		if (mousePos.getX() > bounds.x && mousePos.getX() < bounds.x + bounds.w && mousePos.getY() > bounds.y && mousePos.getY() < bounds.y + bounds.h) {
		
			state = GuiControlState::FOCUSED;

			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
				state = GuiControlState::PRESSED;
			}
			
			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
				NotifyObserver();
			}
		}
		else {
			state = GuiControlState::NORMAL;
		}
		
	}



	return false;
}

bool GuiControlButton::PostUpdate()
{
	if (Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY)
	{
		return true;
	}

	if (state == GuiControlState::OFF) {
		switch (state)
		{
		case GuiControlState::OFF:
			if (offTex)
				Engine::GetInstance().render->DrawTextureForButtons(offTex, bounds.x, bounds.y);
			break;
		}
	}

	if (state != GuiControlState::DISABLED && state != GuiControlState::OFF)
	{
		if (Engine::GetInstance().scene->dibujar)
		{

			switch (state)
			{
			case GuiControlState::OFF:
				if (offTex)
					Engine::GetInstance().render->DrawTextureForButtons(offTex, bounds.x, bounds.y, nullptr, 0.0f);	
				break;
			case GuiControlState::NORMAL:
				if (normalTex)
					Engine::GetInstance().render->DrawTextureForButtons(normalTex, bounds.x, bounds.y, nullptr, 0.0f);
				break;
			case GuiControlState::FOCUSED:
				if (focusedTex)
					Engine::GetInstance().render->DrawTextureForButtons(focusedTex, bounds.x, bounds.y, nullptr, 0.0f);
				break;
			case GuiControlState::PRESSED:
				if (pressedTex)
					Engine::GetInstance().render->DrawTextureForButtons(pressedTex, bounds.x, bounds.y, nullptr, 0.0f);
				break;
			case GuiControlState::ON:
				if (pressedTex)
					Engine::GetInstance().render->DrawTextureForButtons(pressedTex, bounds.x, bounds.y, nullptr, 0.0f);
				break;
			}

			Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);
		}
	}

	return true;
}
