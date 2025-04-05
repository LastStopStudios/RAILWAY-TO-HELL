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
	{"DE2L1T2", 1, 10046, 2160,false},
	{"DT2L1E2", 4, 825, 815,false},
	{"DT2L1E1", 3, 3050, 860,false},
	{"DE1L1T2", 1, 864, 2149,false}
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

	// Whip attack properties
	Animation whipAttack;
	PhysBody* whipAttackHitbox = nullptr;
	bool isWhipAttacking = false;
	bool canWhipAttack = true;
	float whipAttackCooldown = 0.0f;
	SDL_Texture* whipAttackTexture = nullptr;

	Animation jump; 
	SDL_Texture* jumpTexture = nullptr;
	bool isPreparingJump;
	int jumpFrameThreshold;

	Animation walk;
	bool isWalking;

	Animation* currentAnimation = nullptr;
	SDL_Texture* idleTexture = nullptr;  // Attack visual
	bool godMode;
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