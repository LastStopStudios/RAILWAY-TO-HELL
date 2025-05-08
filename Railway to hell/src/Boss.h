#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include <vector>
#include "Box2D/Box2D.h"

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

	void TriggerBossDialog();

	// Load & save 

	void SetDeathInXML();

	void SetAliveInXML();

	void SetSavedDeathToDeathInXML(); // at the moment its not being used

	void SetSavedDeathToAliveInXML();

	void SetEnabled(bool active);

	bool IsEnabled() const { return isEnabled; }

	std::string GetRef() { return ref; }

	void SavePosition(std::string name);

	void ResetLives();

	int GetCurrentLives() { return lives; }
public:

	// Load & save 
	bool pendingDisable = false;
	int DeathValue = 0;
	int SavedDeathValue = 0;

	// death
	int a = 0;
	int kill = 1;
private:
	// Load & save
	std::string enemyID;
	std::string ref;
	bool isEnabled = true;

	// Boss logic
	float initialWalkTimer = 0.0f;
	bool initialWalkComplete = false;
	const float initialWalkDuration = 4.0f; // duration in seconds

	Vector2D enemyPos; 

	float moveSpeed; 
	float patrolSpeed;
	float savedPosX; 
	bool patroling = false; 

	PhysBody* pbodyUpper;
	PhysBody* pbodyLower;

	b2Joint* bodyJoint;

	SDL_RendererFlip flip = SDL_FLIP_NONE; 
	bool isLookingLeft = false; 

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	pugi::xml_node parameters;

	Animation* currentAnimation = nullptr;
	Animation idle, attack, running, hurt, die;

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

	// lives for the boss
	int lives = 12; // for testing try to set it to 2

	int offsetX;
	bool isDying = false;
	bool isDead = false;
	float deathTimer = 0.0f;
	const float deathDelay = 1.0f;

	bool dialogTriggered = false;
	bool battleStarted = false;

	bool ishurt = false;
	
};

