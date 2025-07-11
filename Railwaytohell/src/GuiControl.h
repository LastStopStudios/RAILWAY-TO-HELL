#pragma once

#include "Input.h"
#include "Render.h"
#include "Module.h"

#include "Vector2D.h"

enum class GuiControlType
{
	BUTTON,
	TOGGLE,
	CHECKBOX,
	SLIDER,
	SLIDERBAR,
	COMBOBOX,
	DROPDOWNBOX,
	INPUTBOX,
	VALUEBOX,
	SPINNER
};

enum class GuiControlState
{
	DISABLED,
	NORMAL,
	FOCUSED,
	PRESSED,
	SELECTED,
	ON,
	OFF
};

class GuiControl
{
public:

	// Constructor
	GuiControl(GuiControlType type, int id) : type(type), id(id), state(GuiControlState::NORMAL) {}

	// Constructor
	GuiControl(GuiControlType type, SDL_Rect bounds, const char* text) :
		type(type),
		state(GuiControlState::NORMAL),
		bounds(bounds),
		text(text)
	{
		color.r = 255; color.g = 255; color.b = 255;
		texture = NULL;
	}

	virtual bool Start() { 
		return true; 
	}

	// Called each loop iteration
	virtual bool Update(float dt)
	{
		return true;
	}

	virtual bool PostUpdate()
	{
		return true;
	}

	// 
	void SetTexture(SDL_Texture* tex)
	{
		texture = tex;
		section = { 0, 0, 0, 0 };
	}

	// 
	void SetObserver(Module* module)
	{
		observer = module;
	}

	// 
	void NotifyObserver()
	{
		observer->OnGuiMouseClickEvent(this);
	}

	virtual int GetValue() const { return 0; }

public:

	int id;
	GuiControlType type;
	GuiControlState state;

	std::string text;           
	SDL_Rect bounds;        
	SDL_Color color;        

	SDL_Texture* texture;  
	SDL_Rect section;       

	Module* observer;        
};