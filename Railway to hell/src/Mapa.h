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

	struct MapaQueCargar {
		std::string escena;
		std::string mapardo;
	};
	//list of scenes with their id
	std::vector<MapaQueCargar> mapardo = {
		//sensor id list with info: scene to load, player's x, player's y, fade in and fade out, boss camera
		{"scene","Assets/Textures/mapa/Mapa.png"},//map for scene
		{"scene1","Assets/Textures/mapa/Mapa.png"},//map for scene 1
		{"scene2","Assets/Textures/mapa/Mapa.png"},//map for scene 2
		{"scene3","Assets/Textures/mapa/Mapa.png"},//map for scene 3
		{"scene4","Assets/Textures/mapa/Mapa.png"},//map for scene 4
		{"scene5","Assets/Textures/mapa/Mapa.png"},//map for scene 5
		{"scene6","Assets/Textures/mapa/Mapa.png"},//map for scene 5
		{"scene7","Assets/Textures/mapa/Mapa.png"},//map for scene 8
		{"scene8","Assets/Textures/mapa/Mapa.png"},//map for scene 9
		{"scene9","Assets/Textures/mapa/Mapa.png"},//map for scene 10
		{"scene10","Assets/Textures/mapa/Mapa.png"}//map for scene 11
	};

	struct ZonasPj {
		std::string escena;
		int x1;// firts X of the zone
		int x2; // second X of the zone
		int y1; // first Y of the zone
		int y2; //second Y of the zone
		int playerx;//Player icon X position
		int playery;//Player icon Y position
	};
	//list of scenes with their id
	std::vector<ZonasPj> zonaspj = {
		//sensor id list with info: scene to load, player's x, player's y, fade in and fade out, boss camera
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 1 of the map
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 2 of the map
		{"scene", 0, 2000, 0, 2000, 200, 200},//Zone 3 of the map
		{"scene",0, 2000,  0, 2000, 200, 200},//Zone 4 of the map
		{"scene",0, 2000,  0, 2000, 200, 200},//Zone 5 of the map
	};

private:
	// Private methods
	void ShowMap();
	void LoadMap();

public:
	bool Mostrar;

private:
	//Textures
	SDL_Texture* fondo = nullptr;//background texture
	SDL_Texture* mapa = nullptr;//Map texture
	SDL_Texture* pj = nullptr;//Map texture
	SDL_Texture* cobertura = nullptr;//Black texture

	std::string scene = "";//what scene is now loading
	std::string mapaC = "";//map to show
	std::string pjposition = "";//what scene is now loading
	std::string Zona;//zone of the map where the player is

	int posx, posy;// playerposition
	int w, h;// screen size
	



};

