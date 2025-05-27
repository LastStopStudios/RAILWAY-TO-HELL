#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"


struct SDL_Texture;

class Explosivo : public Entity
{
public:

	Explosivo();
	virtual ~Explosivo();

	bool Awake();

	bool Start();

	bool Update(float dt);
	void Matar();

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:
	//bool dialogo = true;
	float texIsDeath;

	int lives = 1;
	//Death control
	int a = 0;
	int kill = 1;

	//explosive control
	bool cigarro = false, exploto = false, toco = false;
	float explosiveTimer = 0.0f;
	const float Ivolo = 15.0f; //DO NOT TOUCH, if timer is higher when enemy explotes game crash
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

	int dieSFX;
	bool isDying = false;
	bool shouldDestroy = false;
	float deathTimer = 0.0f;
	const float deathDelay = 1.0f;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation hurt;
	Animation explode;
	Animation spot;
	Animation run;
	Animation die;
	PhysBody* pbody;

	bool isSpotting, hasSpotted;
	float explodeOffsetY = 32.0f;
	bool isExploding = false; 

	Pathfinding* pathfinding;
	bool isDead = false;
	bool hasPlayedDeathAnim = false;

	bool ishurt = false;
	//Patroling Control 
	int vez, patrol1, patrol2;

};
