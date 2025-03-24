#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"

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

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);
	Vector2D GetPosition();

private:
	// Private methods
	void DrawPlayer();
	void UpdateMeleeAttack(float dt);

public:
	// Public properties
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	// Audio fx
	int pickCoinFxId;

	PhysBody* pbody;
	float jumpForce = 2.5f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;

private:
	// Private properties
	Animation meleeAttack;       // Attack animation sequence
	bool isAttacking = false;    // Currently attacking flag
	bool canAttack = true;       // Attack availability flag
	float attackCooldown = 0.0f; // Time until next attack allowed
	PhysBody* attackHitbox = nullptr;  // Attack collision area
	SDL_Texture* attackTexture = nullptr;  // Attack visual
	bool facingRight = true;     // Direction facing (true=right)

	bool isDashing = false;
	float dashSpeed = 0.6f;
	float dashCooldownTimer = 0.0f;
	float dashCooldownDuration = 1000.0f; // 1000 second cooldown
	bool canDash = true;	

};