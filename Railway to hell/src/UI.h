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
	void Boss();//Only if there is a boss figth


public:
	bool dialogoOn = false;
	bool figth = false;
	int vidap;

private:
	//Textures
	SDL_Texture* vida = nullptr;//health texture
	SDL_Texture* Evida = nullptr;//Empty health texture
	SDL_Texture* vida2 = nullptr;//health texture
	SDL_Texture* Evida2 = nullptr;//Empty health texturev
	SDL_Texture* amo = nullptr;//stamina texture (Dash)
	SDL_Texture* boss = nullptr;//stamina texture (Dash)

	float posx = 0, posy = 0, posx2=20, posx3 = 40, posxb, posyb;// renders position
	int w=30, h=20, w2, h2, wb, hb;// renders size
	

};

