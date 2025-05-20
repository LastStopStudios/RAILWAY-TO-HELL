#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

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

public:

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
	Animation idle, hurt, die, disparoR, disparoG, salto;

	PhysBody* pbody;
	PhysBody* area;

	Pathfinding* pathfinding;

	bool moving = false;
	bool isdeath = false;
	bool resting = false;
	bool isAttacking = false;

	float attackCooldown = 3000.0f;
	float currentAttackCooldown = 0.0f;
	int attackCounter = 0; 

	float attackDistance = 10.0f;
	bool canAttack = true;

	int lives = 8;

	// jump variables

	bool isJumping = false; // Controlar estado de salto
	float jumpTimer = 0.0f; // Temporizador del salto
	Vector2D jumpStartPos;  // Posición inicial del salto
	Vector2D jumpTargetPos; // Posición objetivo (jugador)
	float jumpDuration = 0.0f; // Duración del salto (en segundos)

	// Función auxiliar para obtener el número total de frames
	int GetTotalFrames() const;
	// Función auxiliar para obtener el índice del frame actual
	int GetCurrentFrameId() const;
};

