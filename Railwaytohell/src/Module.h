#pragma once

#include <string>
#include "pugixml.hpp"

class GuiControl;

class Module
{
public:

	Module() : active(false)
	{}

	void Init()
	{
		active = true;
	}

	// Called before render is available
	virtual bool Awake()
	{
		return true;
	}

	// Called before the first frame
	virtual bool Start()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PreUpdate()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool Update(float dt)
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PostUpdate()
	{
		return true;
	}

	// Called before quitting
	virtual bool CleanUp()
	{
		return true;
	}
	
	//Declare a function to read the XML parameters	
	virtual bool LoadParameters(pugi::xml_node parameters)
	{
		configParameters = parameters;
		return true;
	}

	virtual bool OnGuiMouseClickEvent(GuiControl* control)
	{
		return true;
	}

public:

	std::string name;
	bool active;
	//Declare a pugi::xml_node to store the module configuration parameters
	pugi::xml_node configParameters;

};