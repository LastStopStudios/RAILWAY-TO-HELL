#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Elevators : public Entity
{
public:
	Elevators();
	virtual ~Elevators();

	bool Awake();

	bool Start();

	// Called before all Updates
	bool PreUpdate();

	bool Update(float dt);

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	bool CleanUp();
public:
private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, activated;

	//Add a physics to an door
	PhysBody* pbody;

};

