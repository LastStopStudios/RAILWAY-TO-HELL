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

	bool CleanUp();
private:

public:

private:
};

