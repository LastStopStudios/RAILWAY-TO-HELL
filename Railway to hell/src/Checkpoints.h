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

	void setActivatedToFalse();

	void setToActivatedAnim();

	void setToIdleAnim();

	std::string GetCheckpointType() { return enemyID; } 

	bool GetActivitatedXML() { return activatedXML; }

	void ResetOthersCheckpoints();

public:

	bool isPicked = false;

	bool isActivated = false;

	bool pendingToChangeAnim = false;
	bool actionPressed = false;

private:

	std::string enemyID;
	bool activatedXML = false;
	int sceneForThisCheckpoint = 0;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	int checkpointFX;

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, activating, activated;

	PhysBody* pbody;

};