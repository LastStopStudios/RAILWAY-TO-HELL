#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "Physics.h"


struct SDL_Texture;

class Terrestre : public Entity
{
public:

	Terrestre();
	virtual ~Terrestre();

	bool Awake();

	bool Start();

	bool Update(float dt);
	void Matar();

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetSpecificParameters(pugi::xml_node specificNode) {
		if (specificNode.attribute("x") && specificNode.attribute("y")) {
			position.setX(specificNode.attribute("x").as_int());
			position.setY(specificNode.attribute("y").as_int());

			// Actualizar posici�n del cuerpo f�sico si existe
			if (pbody) {
				b2Vec2 bodyPos(
					METERS_TO_PIXELS(position.getX() + texW / 2),
					METERS_TO_PIXELS(position.getY() + texH / 2)
				);
				pbody->body->SetTransform(bodyPos, 0);
			}
		}
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);



	//Control de dialogos
	//void DialogoOn() { dialogo = true; }//parar enemigos
	//void DialogoOff() { dialogo = false; }//devolver control enemigos

public:
	//bool dialogo = true;
	float texIsDeath;

	int lives = 2;
	//control de muerte
	int a = 0;
	int kill = 1;
private:
	// Update logic
	SDL_RendererFlip flip = SDL_FLIP_NONE; 
	bool isLookingLeft, isLookingRight = false, giro = true;
	float patrolLeftLimit;
	float patrolRightLimit;
	std::string spellType;
	float moveSpeed;
	Vector2D enemyPos;
	bool attacking = false;
	bool falling = false;
	bool isChasing;

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
	Pathfinding* pathfinding;
	bool isDead = false;
	bool hasPlayedDeathAnim = false;
};
