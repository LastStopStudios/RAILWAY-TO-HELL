#pragma once
#include "Module.h"
#include <vector>
#include <string>

struct SDL_Texture;

class UI : public Module
{
public:
	// Public methods
	UI();

	// Destructor
	virtual ~UI();

	// Called before DialogueManager is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();


private:
	void LoadTUi();//loads textures of the UI
	void renderUI();//renders UI


public:
	bool dialogoOn = false;

private:
	//Textures
	SDL_Texture* vida = nullptr;//health texture
	SDL_Texture* stamina = nullptr;//stamina texture (Dash)

	float posx, posy, posx2, posy2;// renders position
	int w, h, w2, h2;// renders size
	

};

