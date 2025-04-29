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

private:
	// Private methods
	void ShowMap();

public:
	bool Mostrar;

private:
	SDL_Texture* fondo = nullptr;//background texture
	std::string scene = "";
};

