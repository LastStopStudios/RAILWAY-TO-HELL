#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "DialogoM.h"
#include "UI.h"

struct SDL_Texture;

class Bufon : public Entity
{
public:

	Bufon();
	virtual ~Bufon();

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

	void ResetPosition();

	int GetCurrentLives() { return lives; }

	int GetInitX() { return initX; }

	int GetInitY() { return initY; }

public:

	// Load & save 
	bool pendingDisable = false;
	int DeathValue = 0;
	int SavedDeathValue = 0;
	bool itemCreated = false;
	// death
	int a = 0;
	int kill = 1;

private:

	// Load & save
	std::string enemyID;
	std::string ref;
	bool isEnabled = true;

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

	int initX, initY;

	pugi::xml_node parameters;

	Animation* currentAnimation = nullptr;
	Animation idle, hurt, die, disparoR, disparoG, jumping, going_up, going_down, impacting;

	PhysBody* pbody;
	PhysBody* jumpAttackArea;

	Pathfinding* pathfinding;

	bool moving = false;
	bool isdeath = false;
	bool resting = false;
	bool isAttacking = false;
	bool isJumpAttacking = false;

	bool phase_One, phase_Two, phase_Three = false;

	float attackCooldown = 4000.0f;
	float currentAttackCooldown = 0.0f;
	int attackCounter = 0; 

	float pathfindingTimer = 0.0f;
	float maxPathfindingTime = 700.0f;

	float attackDistance = 12.0f;
	bool canAttack = true;

	int lives = 2;

	// jump variables

	bool isJumping = false; 
	float jumpTimer = 0.0f; 
	Vector2D jumpStartPos; 
	Vector2D jumpTargetPos; 
	float jumpDuration = 0.0f; 

	// Función auxiliar para obtener el número total de frames
	int GetTotalFrames() const;
	// Función auxiliar para obtener el índice del frame actual
	int GetCurrentFrameId() const;

	bool isDying = false;
	bool isDead = false;
	float deathTimer = 0.0f;
	const float deathDelay = 1.0f;

	bool dialogTriggered = false;
	bool battleStarted = false;

	bool ishurt = false;

	//one hit colision
	bool Hiteado = false;

	bool changeMusicBoss = false;
};

