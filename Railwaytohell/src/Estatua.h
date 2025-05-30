#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Estatua : public Entity
{
public:

	Estatua();
	virtual ~Estatua();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	bool CleanUp();

private:

public:

private:
	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, activated;

	PhysBody* pbody;
};

