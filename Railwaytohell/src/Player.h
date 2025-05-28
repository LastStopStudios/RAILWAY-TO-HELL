#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"
#include <vector>
#include <chrono>
#include <thread>

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
		float x;
		float y;
		bool fade;
		bool CamaraBoss;
	};
	//list of scenes with their id
	std::vector<EscenaQueCargar> escenas = {
	//sensor id list with info: scene to load, player's x, player's y, fade in and fade out, boss camera
	{"DT2L1EC",6,3072,658,true,false},
	{"DT2L1DE2L1",3,700,600,true,false},
	{"DT2L1DTL1L2",7,3072,3859,true,false},
	{"IE1L1IT1L1",10,497,4400,true,false},
	{"DE2L1DT2L1",1,10114,2122,true,false},
	{"DE2L2DT2L2",8,10113,3296,true,false},
	{"IE1L2IT1L2",11,446,3211,true,false},
	{"ECDT2L1",1,690,2137,true,false},
	{"ECIT1L1",10,10100,2048,true,false},
	{"ECDT2L2",8,775,2532,true,false},
	{"ECIT1L2",11,9812,1680,true,false}, 
	{"DTL1L2DT2L1",1,5561,5984,true,false},
	{"DTL1L2DT2L2",8,7818,2564,true,false},
	{"DT2L2EC",6,3072,1640,true,false},
	{"DT2L2DTL1L2",7,6699,4172,true,false},
	{"DT2L2DE2L2",4,724,422,true,false},
	{"ITL1L2IT1L1",10,3962,2610,true,false},
	{"ITL1L2IT1L2",11,4697,5570,true,false},
	{"IT1L1EC",6,750,689,true,false},
	{"IT1L1IE1L1",2,3097,729,true,false},
	{"IT1L1ITL1L2",9,6257,2121,true,false},
	{"IT1L2EC",6,779,1652,true,false},
	{"IT1L2IE1L2",5,3123,665,true,false},
	{"IT1L2ITL1L2",9,2648,2414,true,false},
	{"ECP1AP2",6,2571,1633,true,false},// Stairs EC
	{"ECP2AP1",6,2542,663,true,false},// Stairs EC
	{"ECP2AP3",6,1098,2622,true,false},// Stairs EC
	{"ECP3AP2",6,1101,1615,true,false},// Stairs EC
	{"DE2L1BOSS1",3,1381,1573,true,true},// Elevator first floor first boss
	{"BOSS1DE2L1",3,2624,704,true,false},// Elevator second floor first boss
	{"IE1L2BOSS2",5,1400,1550,true,true},// Elevator first floor SECOND boss
	{"BOSS2IE1L2",5,1140,575,true,false},// Elevator second floor SECOND boss
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
	//health
	void hit();

	int GetLastCheckpointScene();

public:
	
	void ResetLives() {
		lives = 5;
	};

	void ResetToInitPosition();
	void SaveInitialPosition();
	float Scene1InitX, Scene1InitY;
	float Scene2InitX, Scene2InitY;
	float Scene3InitX, Scene3InitY;
	float Scene4InitX, Scene4InitY;
	float Scene5InitX, Scene5InitY;
	float Scene6InitX, Scene6InitY;
	float Scene7InitX, Scene7InitY;
	float Scene8InitX, Scene8InitY;
	float Scene9InitX, Scene9InitY;
	float Scene10InitX, Scene10InitY;
	float Scene11InitX, Scene11InitY;

	

private:
	// Private methods
	void DrawPlayer();
	void UpdateMeleeAttack(float dt);
	void HandleJump(float dt);
	void HandleSceneSwitching();
	void HandleHurt(float dt);
	void HandleDash(b2Vec2& velocity, float dt);
	void HandleMovement(b2Vec2& velocity);
	void UpdateWhipAttack(float dt);
	void HandleBallAttack(float dt);
	void HandlePickup(float dt);
	void HandleDeath(float dt);
	void HandleWakeup(float dt);
	void Idle();
	void HitWcooldown(float dt);
	void Abyss();
	void ResetPlayerPosition();
private:
	bool waitForHurtAnimation = false;
	bool pendingAbyssTeleport = false;
	int abyssTeleportX = 9632;
	int abyssTeleportY = 1696;
	bool canHurtAbyss = true;
	// Public properties
	SDL_Texture* texture = NULL;
	int texW, texH;
	float leftOffsetX;
	float rightOffsetX;
	float heightOffset;

	float fallingTimer = 0.0f;
	bool isTransitioningToFalling = false;

	//Scene change
	bool NeedSceneChange = false;
	bool Fade;
	bool BossCam;
	int sceneToLoad = -1;
	float Playerx, Playery;

	bool changeMusicCaronte = false;

	//Dialogues
	bool NeedDialogue = false;
	std::string Id;

	// Audio fx
	int pickCoinFxId, punchFX, stepFX, diedFX, hurtFX,dashFX,whipFX,fallFX,jumpFX, itemFX;

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
		leverTwo = false;
		leverThree = false;
		leverFour = false;
	}

	bool returnCanOpenDoor() { return canOpenDoor; }
	bool returnLeverOne() { return leverOne; }
	bool returnLeverTwo() { return leverTwo; }
	bool returnLeverThree() { return leverThree; }
	bool returnLeverFour() { return leverFour; }
	void RestoreFullStats();
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

	// Wakeup properties
	Animation wakeupAnim;
	SDL_Texture* wakeupTexture = nullptr;
	bool isWakingUp, hasWokenUp;
	bool hasWakeupStarted;
	float wakeupTimer;

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

	// Throw
	Animation throwAnim;
	SDL_Texture* throwTexture;
	bool isThrowing = false;
	float throwTimer = 0.0f;
	bool ballToShoot = false;
	bool hasThrownBall = false;

	// items booleans
	bool Dash = false;
	bool WhipAttack = false;
	bool BallAttack = false;

	// door and lever booleans
	bool canOpenDoor = false;
	bool leverOne = false;
	bool leverTwo = false;
	bool leverThree = false;
	bool leverFour = false;

	// lives
	int lives = 5;

	//Ice platform
	bool resbalar, giro, primero = true, primerob = true;
	float icev = 3;
	float a = 0, b = 0, anta = 0.5, antb = 0.5, dificultty = 20 ;

	//double jump
	bool doubleJump = false;       // Whether double jump ability is enabled
	bool canDoubleJump;    // Whether player can perform a double jump in the current jump sequence
	int jumpCount;         // Track how many jumps have been performed in sequence

	bool collidingWithEnemy = false; //if is true, player cant ball attack 
	int ballCounter = 3; // Track how many balls does the player have left
	float ballCooldown = 3000.0f; // Time until ball recharge

	
	bool isFallingInAbyss = false; //if player touches the abyss
	//colliding with enemies
	std::chrono::time_point<std::chrono::steady_clock> lastHitTime;
	bool isTouchingEnemy = false;
	bool wasTouchingEnemy = false;
	float damageCooldown = 0.0f;
	const float DAMAGE_COOLDOWN_TIME = 1.0f; // 1 segundo entre daños

	bool tocado = false;//Enemy collides with player
	bool first = true;// first time colision
	float hitCooldown = 2000.0f;// 2 seconds cooldown timer

	// Delay time in seconds before damage animation (0,5s)
	float hurtDelay = 500.0f;      
	float currentHurtDelay = 0.0f; 
	bool isHurtDelayed = false;  
	bool freezeWhileHurting = false;

	bool ballhurt = false; //if player is hurt by ball attack
	bool bufonjumphurt = false; //if player is hurt by bufon jump attack



	Animation slide;
	SDL_Texture* slideTexture = nullptr;

};