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

	void PopUps();//Show PopUps when items are pick up or puzzle is resolved

public:
	bool dialogoOn = false;
	bool PopeadaTime = false;//PopUp
	bool figth = false, figth2 = false, figth3 = false, fase1 = false, fase2 = false, fase3 = false;//Bosses figth and phases
	int vidap, vidab1, vidab2, vidab3;//Entities health
	int item;

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
	SDL_Texture* Evida = nullptr;//Empty Boss health texture

	//SDL_Texture* amo = nullptr;//stamina texture (Dash)
	//SDL_Texture* boss = nullptr;//stamina texture (Dash)
	
	// PopUps
	SDL_Texture* Ball = nullptr;// Ball keyboard texture
	SDL_Texture* Ball2 = nullptr;// Ball controller texture
	SDL_Texture* Dash = nullptr;// Dash  keyboard texture
	SDL_Texture* Dash2 = nullptr;// Dash controller texture
	SDL_Texture* DJump = nullptr;// Double Jump keyboard texture
	SDL_Texture* DJump2 = nullptr;// Double controller texture
	SDL_Texture* Whip = nullptr;// Whip  keyboard texture
	SDL_Texture* Whip2 = nullptr;// Whip controller texture
	SDL_Texture* Puzzle = nullptr;// Puzzle complete texture
	//UI sizes
	int w = 40, h = 40, w2, h2, wb=60, hb = 60;
	//UI positions
	float posy = 0, posx = 0, posx2=30, posx3 = 60, posx4 = 90, posx5 = 120, posxb, posyb;// renders player position
	float posy2 = 700, bposx = 400, bposx2 = 460, bposx3 = 520, bposx4 = 580, bposx5 = 640, bposx6 = 700, bposx7 = 760, bposx8 = 820, bposx9 = 880, bposx10 = 940;//render boss position

};

