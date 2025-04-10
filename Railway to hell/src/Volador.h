#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Volador : public Entity
{
public:

	Volador();
	virtual ~Volador();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	Vector2D GetLastPosition() const {
		return Lastposition;
	}

	void ResetPath();

	void Matar();

	std::string GetType() const { return "volador"; }

public:
	float texIsDeath;

	int lives = 2;
	//control de muerte
	int a = 0;
	int kill = 1;

private:

	Vector2D Lastposition;
	Vector2D enemyPos;
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	bool isLookingLeft, isLookingRight = false, giro = true;;
	bool isAttacking;
	bool isTouchingCollision = false;
	float moveSpeed;          // Enemy movement speed
	float minX, maxX;         // Movement limits on the X-axis
	bool movingRight;         // Current direction of the enemy
	bool chasingPlayer;       // Indicator if the enemy is chasing the player
	std::vector<Vector2D> path; // Path calculated towards the player
	bool playerNotFound;

	int dieSFX;
	bool isDying = false;
	bool shouldDestroy = false;
	float deathTimer = 0.0f;
	const float deathDelay = 1.0f;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation die;
	Animation hurt;
	PhysBody* pbody;
	PhysBody* area;
	Pathfinding* pathfinding;
	bool searching = false;
	bool followingPath = false;
	float searchTime = 0.0f;
	const float maxSearchTime = 3000.0f;
	int texRadius;
	bool isDead = false;
	bool hasPlayedDeathAnim = false;
};

