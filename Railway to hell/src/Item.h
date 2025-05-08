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

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetItemType(const std::string& type) { itemType = type; }

	const std::string& GetItemType() const { return itemType; }

	// Load & save 

	void SetDeathInXML();

	void SetAliveInXML();

	void SetSavedDeathToDeathInXML(); // at the moment its not being used

	void SetSavedDeathToAliveInXML();

	void SetCreatedTrueInXML();

	void SetCreatedFalseInXML();

	void SetEnabled(bool active);

	bool IsEnabled() const { return isEnabled; }

	std::string GetRef() { return ref; }

	void SavePosition(std::string name);

public:

	// Load & save 
	bool pendingDisable = false;
	int DeathValue = 0;
	int SavedDeathValue = 0;


	bool isPicked = false;

private:

	// Load & save
	std::string enemyID;
	std::string ref;
	bool isEnabled = true;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;

	//Add a physics to an item
	PhysBody* pbody;

	std::string itemType;
};
