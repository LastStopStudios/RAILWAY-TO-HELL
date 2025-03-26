#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"
#include <vector>

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);
	Vector2D GetPosition();

	struct EscenaQueCargar {
		std::string escena;
		int id;
	};

	std::vector<EscenaQueCargar> escenas = {//lista de escenas con su id
	{"S1S2", 2},
	{"S2S3", 3},
	{"S3S1", 1},
	{"S1S3", 3},
	{"S3S2", 2},
	{"S2S1", 1}
	};
	

public:
	bool needSceneChange = false;
	int sceneToLoad = -1;
	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;
	std::string esc;
	//Audio fx
	int pickCoinFxId, sensorId = 0;
	int valorid;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 2.5f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
};