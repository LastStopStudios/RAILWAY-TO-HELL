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
	bool PostUpdate();
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
		bool CamaraBoss;
	};
	//list of scenes with their id
	std::vector<EscenaQueCargar> escenas = {
	//sensor id list with info: scene to load, player's x, player's y, fade in and fade out, boss camera
	{"DE2L1T2",1,10046,2160,false,false},
	{"DT2L1E2",3,825,815,false,false},
	{"DT2L1E1",2,3050,860,false,false},
	{"DE1L1T2",1,864,2149,false,false},
	{"DE2L1BOSS1",3,1840,1784,true,true},
	{"BOSS1DE2L1",3,1840,815,true,false},
	};
	//elevator control
	bool TocandoAs = false;
	bool Bloqueo = false;
	void Ascensor();
	void DesbloquearSensor();
	void BloquearSensor();
	//Dialog control
	void DialogoOn() { dialogo = true;}//stop player
	void DialogoOff() { dialogo = false;}//return control player


private:
	// Private methods
	void DrawPlayer();
	void UpdateMeleeAttack(float dt);
	void HandleJump();
	void HandleSceneSwitching();
	void HandleHurt(float dt);
	void HandleDash(b2Vec2& velocity, float dt);
	void HandleMovement(b2Vec2& velocity);
	void UpdateWhipAttack(float dt);
	void HandlePickup(float dt);
	void HandleDeath(float dt);


public:
	// Public properties
	SDL_Texture* texture = NULL;
	int texW, texH;
	float leftOffsetX;
	float rightOffsetX;
	float heightOffset;

	//Scene change
	bool NeedSceneChange = false;
	bool Fade;
	bool BossCam;
	int sceneToLoad = -1;
	int Playerx, Playery;

	//Dialogues
	bool NeedDialogue = false;
	std::string Id;

	// Audio fx
	int pickCoinFxId, punchFX, stepFX;

	PhysBody* pbodyUpper;
	PhysBody* pbodyLower;

	b2Joint* bodyJoint; 

	bool isJumping = false; // Flag to check if the player is currently jumping

	pugi::xml_node parameters;
	Animation idle;

public:

	void ResetDoorAndLeverStates() {
		canOpenDoor = false;
		leverOne = false;
	}

	bool returnCanOpenDoor() { return canOpenDoor; }
	bool returnLeverOne() { return leverOne; }
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

	Animation jump, falling, recovering; 
	SDL_Texture* jumpTexture = nullptr;
	SDL_Texture* fallingTexture = nullptr;
	SDL_Texture* recoveringTexture = nullptr;
	bool isFalling;
	bool isRecovering;
	float recoveringTimer;
	const float recoveringDuration = 0.3f;
	bool isPreparingJump;
	int jumpFrameThreshold;

	Animation dash;
	SDL_Texture* dashTexture = nullptr;
	float dashCooldown;
	float dashDirection;   // Dash direction (1.0f for right, -1.0f for left)
	float dashSpeed;       // Dash speed
	float dashDistance;    // Total dash distance // Dash direction (1.0f for right, -1.0f for left)
	int dashFrameCount;  // Frame counter for dash

	float originalGravityScale;

	Animation walk;
	bool isWalking;
	SDL_Texture* walkTexture;
	float runSoundTimer = 0.0f;
	float runSoundInterval = 0.5f;

	Animation hurt;
	bool isHurt = false;
	bool hasHurtStarted = false;
	bool hurted = false;
	SDL_Texture* hurtTexture = nullptr;

	// Pickup properties
	Animation pickupAnim;
	SDL_Texture* pickupTexture = nullptr;
	bool isPickingUp = false;
	bool hasPickupStarted = false;

	Animation* currentAnimation = nullptr;
	SDL_Texture* idleTexture = nullptr;  // Attack visual
	bool godMode;
	// Dash properties
	bool isDashing = false;
	bool canDash = true;
	int totalDashesAvailable = 3;    // Dashes currently available
	int maxTotalDashes = 3;          // Maximum number of dashes allowed
	float dashRechargeTimer = 0.0f;  // Timer for complete recharge
	float dashFullRechargeTime = 5000.0f; // Time to recharge all dashes
	
	// Death properties
	Animation death;
	SDL_Texture* deathTexture = nullptr;
	bool isDying = false;
	bool hasDeathStarted = false;
	bool hasDied = false;

	// items booleans
	bool Dash = false;
	bool WhipAttack = false;
	// door and lever booleans
	bool canOpenDoor = false;
	bool leverOne = false;

	int lives = 3;


};