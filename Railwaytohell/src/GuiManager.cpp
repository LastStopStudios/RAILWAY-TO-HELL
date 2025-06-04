#include "GuiManager.h"
#include "Engine.h"
#include "Textures.h"

#include "CheckBox.h"
#include "GuiControlSlider.h"
#include "GuiControlButton.h"
#include "Audio.h"

GuiManager::GuiManager() :Module()
{
	name = "guiManager";
}

GuiManager::~GuiManager() {}

bool GuiManager::Start()
{
	return true;
}

//Implement CreateGuiControl function that instantiates a new GUI control and add it to the list of controls
GuiControl* GuiManager::CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds)
{
	GuiControl* guiControl = nullptr;

	//Call the constructor according to the GuiControlType
	switch (type)
	{
	case GuiControlType::BUTTON:
		guiControl = new GuiControlButton(id, bounds, text);
		break;
	case GuiControlType::CHECKBOX:
		guiControl = new CheckBox(id, bounds, text);
		break;
	case GuiControlType::SLIDER:
		guiControl = new GuiControlSlider(id, bounds, text, sliderBounds.x, sliderBounds.y);
		break;
	}


	//Set the observer
	guiControl->observer = observer;

	// Created GuiControls are add it to the list of controls
	guiControlsList.push_back(guiControl);

	// Initialize the control
	if (guiControl != nullptr)
	{
		guiControl->Start();
	}

	return guiControl;
}

bool GuiManager::Update(float dt)
{	
	for (const auto& control : guiControlsList)
	{
		control->Update(dt);
	}

	return true;
}

bool GuiManager::PostUpdate()
{
	return true;
}

void GuiManager::DrawOverlay() {
	// Draw all the controls
	for (const auto& control : guiControlsList) {
		control->PostUpdate();
	}
}

bool GuiManager::CleanUp()
{
	for (const auto& control : guiControlsList)
	{
		delete control;
	}

	return true;
}



