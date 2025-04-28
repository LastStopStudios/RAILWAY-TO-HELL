#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Projectiles : public Entity
{
public:

	Projectiles();
	virtual ~Projectiles();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void SetDirection(bool direction);
	void SetDirectionUp(bool direction);

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetSpellType(const std::string& type) { projectileType = type; }

	const std::string& GetSpellType() const { return projectileType; }

	void startImpactAnimation();

private:
	float moveSpeed;

	bool isPicked = false;

	int texW, texH, texRadius;

	bool isLookingLeft = false;
	bool isLookingUp = false;

	bool isImpacting = false;

	bool speedingUp = true;
private:


	SDL_Texture* texture;

	const char* texturePath;

	Animation* currentAnimation = nullptr;
	Animation idle, speedingUpp, moving, impact;

	pugi::xml_node parameters;
	PhysBody* pbody;

	std::string projectileType;

};