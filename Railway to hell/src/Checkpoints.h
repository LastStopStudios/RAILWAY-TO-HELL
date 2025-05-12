#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Checkpoints : public Entity
{
public:

	Checkpoints();
	virtual ~Checkpoints();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void setActivatedToTrue(int scene);

	void setToActivatedAnim();

	std::string GetCheckpointType() { return enemyID; } 

	bool GetActivitatedXML() { return activatedXML; }

public:

	bool isPicked = false;

	bool isActivated = false;

private:

	std::string enemyID;
	bool activatedXML = false;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	int checkpointFX;

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, activating, activated;

	PhysBody* pbody;

};