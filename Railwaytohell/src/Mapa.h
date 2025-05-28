#pragma once
#include "Module.h"
#include <vector>
#include <string>


struct SDL_Texture;

class Mapa : public Module
{
public:
	// Public methods
	Mapa();

	// Destructor
	virtual ~Mapa();

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

	//error control
	void DialogoOn();
	void DialogoOff();

	//XML
	void CoverXML();

	//Player on map
	struct ZonasPj {
		std::string escena;
		float x1;// firts X of the zone smaller than player position
		float x2; // second X of the zone bigger than player position
		float y1; // first Y of the zone smaller than player position
		float y2; //second Y of the zone bigger than player position
		float playerx;//Player's icon X position
		float playery;//Player's icon Y position
	};
	//list of scenes with their id
	std::vector<ZonasPj> zonaspj = {
		//sensor id list with info: scene to load, player's x, player's y, fade in and fade out, boss camera
		{"scene", 0, 2506.67, 0, 2320.0, 200, 200},//Zone 1 of the scene
		{"scene", 2530.67, 3827.00, 10.00, 2317.33, 400, 200},//Zone 2 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 3 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 4 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 5 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 6 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 7 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 8 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 9 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 10 of the scene
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 11 of the scene
		{"scene2", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 2
		{"scene2", 0, 2000, 0, 2000, 200, 200},//Zone 2 of the scene 2
		{"scene3", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 3
		{"scene3", 0, 2000, 0, 2000, 200, 200},//Zone 2 of the scene 3
		{"scene4", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 4
		{"scene5", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 5
		{"scene6", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 6
		{"scene7", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 7
		{"scene8", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 8
		{"scene9", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 9
		{"scene10", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 10
		{"scene11", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the scene 11
	};

	//black Cover
	struct Negro {
		std::string escena;
		float px;// firts X of the zone smaller than player position
		float py; // second X of the zone bigger than player position
		float tmx; // first Y of the zone smaller than player position
		float tmy; //second Y of the zone bigger than player position
		bool visible;
		int num;
	};

	std::vector<Negro> negro = {
		{"scene", 500, 200, 200, 200, false, 1},//covering scene 1
		{"scene2", 1000, 500, 200, 200, false, 2},//covering scene 2
		{"scene3", 1000, 500, 200, 200, false, 3},//covering scene 3
		{"scene4", 1000, 500, 200, 200, false, 4},//covering scene 4
		{"scene5", 1000, 500, 200, 200, false, 5},//covering scene 5
		{"scene6", 1000, 500, 200, 200, false, 6},//covering scene 6
		{"scene7", 1000, 500, 200, 200, false, 7},//covering scene 7
		{"scene8", 1000, 500, 200, 200, false, 8},//covering scene 8
		{"scene9", 1000, 500, 200, 200, false, 9},//covering scene 9
		{"scene10", 1000, 500, 200, 200, false, 10},//covering scene 10
		{"scene11", 1000, 500, 200, 200, false, 11}//covering scene 11
	};

private:
	// Private methods
	void ShowMap();
	void LoadMap();

	void Remember();// Control of all the remembers

public:
	bool Mostrar;
	bool dialogoOn = false;

	bool remember1, remember2, remember3, remember4, remember5, remember6, remember7, remember8;

private:
	//Textures
	SDL_Texture* fondo = nullptr;//background texture
	SDL_Texture* mapa = nullptr;//Map texture
	SDL_Texture* pj = nullptr;//Map texture
	SDL_Texture* MNegro = nullptr;//Black texture
	SDL_Texture* MNegro2 = nullptr;//Black texture
	SDL_Texture* MNegro3 = nullptr;//Black texture
	SDL_Texture* MNegro4 = nullptr;//Black texture
	SDL_Texture* MNegro5 = nullptr;//Black texture
	SDL_Texture* MNegro6 = nullptr;//Black texture
	SDL_Texture* MNegro7 = nullptr;//Black texture
	SDL_Texture* MNegro8 = nullptr;//Black texture
	SDL_Texture* MNegro9 = nullptr;//Black texture
	SDL_Texture* MNegro10 = nullptr;//Black texture
	SDL_Texture* MNegro11 = nullptr;//Black texture
	SDL_Texture* MNegro12 = nullptr;//Black texture

	std::string scene = "";//what scene is now loading
	std::string pjposition = "";//what scene is now loading
	std::string Zona = "";//zone of the map where the player is

	//Remember
	SDL_Texture* Recuerdo1 = nullptr;// Remember 1 texture
	SDL_Texture* Recuerdo2 = nullptr;// Remember 2 texture 
	SDL_Texture* Recuerdo3 = nullptr;// Remember 3 texture 
	SDL_Texture* Recuerdo4 = nullptr;// Remember 4 texture 
	SDL_Texture* Recuerdo5 = nullptr;// Remember 5 texture 
	SDL_Texture* Recuerdo6 = nullptr;// Remember 6 texture 
	SDL_Texture* Recuerdo7 = nullptr;// Remember 7 texture 
	SDL_Texture* Recuerdo8 = nullptr;// Remember 8 texture 
	SDL_Texture* VRecuerdo = nullptr;// Remember Hole texture 

	float posx, posy;// icon player position
	int w, h, i = 1;//renders size

};

