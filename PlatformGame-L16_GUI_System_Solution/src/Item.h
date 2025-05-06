#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Item : public Entity
{
public:

	Item();
	virtual ~Item();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetDeathInXML();

	void SetAliveInXML();

	void SetSavedDeathToDeathInXML(); // at the moment its not being used

	void SetSavedDeathToAliveInXML();

	void SetEnabled(bool active);

	bool IsEnabled() const { return isEnabled; }

	int GetType() { return type; }

public:

	bool pendingDisable = false;
	int DeathValue = 0;
	int SavedDeathValue = 0;
	bool isPicked = false;

private:

	int type;
	bool isEnabled = true;
	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	std::string enemyID;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
