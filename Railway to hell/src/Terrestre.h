#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"


struct SDL_Texture;

class Terrestre : public Entity
{
public:

	Terrestre();
	virtual ~Terrestre();

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

	//Control de dialogos
	//void DialogoOn() { dialogo = true; }//parar enemigos
	//void DialogoOff() { dialogo = false; }//devolver control enemigos

public:
	//bool dialogo = true;
private:
	// Update logic
	SDL_RendererFlip flip = SDL_FLIP_NONE; 
	bool isLookingLeft, isLookingRight = false, giro = true;
	float patrolLeftLimit;
	float patrolRightLimit;
	std::string spellType;
	float moveSpeed;
	Vector2D enemyPos;
	bool attacking = false;
	bool falling = false;
	bool isChasing;
	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	PhysBody* pbody;
	Pathfinding* pathfinding; 

};
