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
	};

private:
	// Private methods
	void ShowMap();
	void LoadMap();

public:
	bool Mostrar;

private:
	SDL_Texture* fondo = nullptr;//background texture
	SDL_Texture* mapa = nullptr;//Map texture
	SDL_Texture* pj = nullptr;//Map texture
	SDL_Texture* cobertura = nullptr;//Black texture
	std::string scene = "";//what scene is now loading
	std::string mapaC = "";//map to show
	std::string pjposition = "";//what scene is now loading
	int posx, posy;// playerposition
	int w, h;// screen size



};

