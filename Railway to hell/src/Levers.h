#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Levers : public Entity
{
public:

	Levers();
	virtual ~Levers();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetLeverType(const std::string& type) { leverType = type; }

	const std::string& GetLeverType() const { return leverType; }

	bool returnLeverActivated() const { return Lever_Door_Activated; }

public:

	bool Activated = false;
	bool Lever_Door_Activated = false;

private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, lever_activated;

	//Add a physics to an door
	PhysBody* pbody;

	std::string leverType;
};
