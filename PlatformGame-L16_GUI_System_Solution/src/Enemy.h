#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"


struct SDL_Texture;

class Enemy : public Entity
{
public:

	Enemy();
	virtual ~Enemy();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

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
	PhysBody* pbody;
	Pathfinding* pathfinding;
};
