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
	void PJs(); //render player lives
	void Boss1();//Only if there is a boss figth


public:
	bool dialogoOn = false;
	bool figth = false;
	int vidap, vidab;

private:
	//Textures
	//Full
	SDL_Texture* vida = nullptr;//health texture
	SDL_Texture* Evida = nullptr;//Empty health texture
	SDL_Texture* vidapl = nullptr;//health texture
	//Empty
	SDL_Texture* vida2 = nullptr;//health texture
	SDL_Texture* vidapl2 = nullptr;//health texture
	SDL_Texture* Evida2 = nullptr;//Empty health texturev
	SDL_Texture* amo = nullptr;//stamina texture (Dash)
	SDL_Texture* boss = nullptr;//stamina texture (Dash)
	//UI sizes
	int w = 40, h = 40, w2, h2, wb=60, hb = 40;
	//UI positions
	float posy = 0, posx = 0, posx2=30, posx3 = 60, posx4 = 90, posx5 = 120, posxb, posyb;// renders player position
	float posy2 = 700, bposx = 500, bposx2 = 540, bposx3 = 580, bposx4 = 620, bposx5 = 660, bposx6 = 700;//render boss 1 position
	
	

};

