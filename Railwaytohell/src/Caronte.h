#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Caronte : public Entity
{
public:

	Caronte();
	virtual ~Caronte();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);
	//Vector2D GetPosition();

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

private:
	// Gestión de la secuencia de muerte
	bool isDying = false;
	float deathTimer = 0.0f;
	bool candie = false;
	bool isattacking = false;
	bool isdeath = false;
	bool canAttack = true;
	bool attacked = false;
	bool attackOnCooldown = false;

	float attackCooldown = 3000.0f;    
	float currentAttackCooldown = 0.0f;
	//dialogue
	bool done;


private:

	SDL_Texture* texture;
	bool deathAnimationPlaying = false;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle, attack, hurt, die;

	// physics from box2d
	PhysBody* pbody;
	PhysBody* AttackSensorArea;
	PhysBody* AttackArea;
};