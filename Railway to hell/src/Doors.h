#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Doors : public Entity
{
public:

	Doors();
	virtual ~Doors();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetDoorType(const std::string& type) { doorType = type; }

	const std::string& GetDoorType() const { return doorType; }

public:

	bool Activated = false;

private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, activated;

	//Add a physics to an door
	PhysBody* pbody;
	PhysBody* pbody2;

	std::string doorType;
};
