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

	// Método para dibujar el Bufón en pantalla
	void Draw();

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
	pugi::xml_node parameters;
	int texW, texH;
	PhysBody* pbody;
	Pathfinding* pathfinding;
	SDL_Texture* texture;
	Animation idle, die, hurt;
	Animation* currentAnimation = nullptr;

	// Función auxiliar para obtener el número total de frames
	int GetTotalFrames() const;
	// Función auxiliar para obtener el índice del frame actual
	int GetCurrentFrameId() const;
};