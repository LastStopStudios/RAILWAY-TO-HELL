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
	void Boss1();//Only if there is a figth with Boss 1
	void Boss2();//Only if there is a figth with Boss 2
	void Boss3();//Only if there is a figth with Boss 3


public:
	bool dialogoOn = false;
	bool figth = false, figth2 = false, figth3 = false;
	int vidap, vidab1, vidab2, vidab3;//Entities health

	//to get boss healt put in boss's update this
	/* if (Engine::GetInstance().ui->*figth with number* == true) {
        //UI Lives
        Engine::GetInstance().ui->*vidab with same figth number* = lives;
    }*/

private:
	//Textures
	//Full
	SDL_Texture* vidaB1 = nullptr;// Boss health texture
	SDL_Texture* vidaB2 = nullptr;// Boss health texture
	SDL_Texture* vidaB3 = nullptr;// Boss health texture
	SDL_Texture* vidaB4 = nullptr;// Boss health texture
	SDL_Texture* vidapl = nullptr;//Player health texture
	//Empty
	SDL_Texture* vidapl2 = nullptr;//Empty Player health texture
	SDL_Texture* Evida = nullptr;//Empty Boss health texturev
	//SDL_Texture* amo = nullptr;//stamina texture (Dash)
	//SDL_Texture* boss = nullptr;//stamina texture (Dash)
	//UI sizes
	int w = 40, h = 40, w2, h2, wb=60, hb = 60;
	//UI positions
	float posy = 0, posx = 0, posx2=30, posx3 = 60, posx4 = 90, posx5 = 120, posxb, posyb;// renders player position
	float posy2 = 700, bposx = 540, bposx2 = 600, bposx3 = 660, bposx4 = 720, bposx5 = 780, bposx6 = 840;//render boss position
	
	

};

