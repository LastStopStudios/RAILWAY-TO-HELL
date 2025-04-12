#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Boss : public Entity
{
public:

	Boss();
	virtual ~Boss();

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
	
	int a = 0;
	int kill = 1;
private:

	Vector2D enemyPos; 

	float moveSpeed; 
	float patrolSpeed;
	float savedPosX; 
	bool patroling = false; 

	SDL_RendererFlip flip = SDL_FLIP_NONE; 
	bool isLookingLeft = false; 

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	pugi::xml_node parameters;

	Animation* currentAnimation = nullptr;
	Animation idle, attack, running, die;

	PhysBody* pbody;
	PhysBody* area;

	Pathfinding* pathfinding;

	bool moving = false;
	bool isdeath = false;
	bool resting = false;
	bool isAttacking = false;

	float attackCooldown = 3000.0f; 
	float currentAttackCooldown = 0.0f;

	float attackDistance = 7.0f;
	bool canAttack = true;

	int lives = 8;
	int offsetX;
	bool isDying = false;
	bool isDead = false;
	float deathTimer = 0.0f;
	const float deathDelay = 1.0f;
};

