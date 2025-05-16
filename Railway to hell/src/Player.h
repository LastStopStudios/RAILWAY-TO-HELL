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
		int x;
		int y;
		bool fade;
	};
	//lista de escenas con su id
	std::vector<EscenaQueCargar> escenas = {
	{"S1S2", 2, 219, 654,false},
	{"S2S3", 3, 173, 648,false},
	{"S3S1", 1, 351, 409,false},
	{"S1S3", 3, 760, 628,false},
	{"S3S2", 2, 1817, 640,false},
	{"S2S1", 1, 1800, 642,false}
	};
	//Control de dialogos
	void DialogoOn() { dialogo = true;}//parar player
	void DialogoOff() { dialogo = false;}//devolver control player


private:
	// Private methods
	void DrawPlayer();
	void UpdateMeleeAttack(float dt);
	void HandleJump();
	void HandleSceneSwitching();
	void HandleDash(b2Vec2& velocity, float dt);
	void HandleMovement(b2Vec2& velocity);
	void UpdateWhipAttack(float dt);
	void InitializeAnimations(); 

public:
	// Public properties
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	//Scene change
	bool NeedSceneChange = false;
	bool Fade;
	int sceneToLoad = -1;
	int Playerx, Playery;

	//Dialogues
	bool NeedDialogue = false;
	std::string Id;
	bool dialogo = false;

	// Audio fx
	int pickCoinFxId;

	PhysBody* pbody;
	float jumpForce = 2.5f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping

	pugi::xml_node parameters;
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

	bool doubleJump = false;    // Whether double jump ability is enabled
	bool canDoubleJump = false; // Whether player can perform a double jump in the current jump sequence
	int jumpCount = 0;          // Track how many jumps have been performed in sequence

	// Whip attack properties
	Animation whipAttack;
	PhysBody* whipAttackHitbox = nullptr;
	bool isWhipAttacking = false;
	bool canWhipAttack = true;
	float whipAttackCooldown = 0.0f;
	SDL_Texture* whipAttackTexture = nullptr;

	Animation* currentAnimation = nullptr;
	SDL_Texture* idleTexture = nullptr;  // Attack visual

	// Dash properties
	bool isDashing = false;
	float dashSpeed = 0.6f;
	float dashCooldownTimer = 0.0f;
	float dashCooldownDuration = 1.0f; // 1 second cooldown
	bool canDash = true;
	float dashDuration = 0.2f; // Duration of the dash
	
	bool Dash = false;
	bool WhipAttack = false;
	bool canOpenDoor = false;
};