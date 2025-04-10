#pragma once

#include "Module.h"
#include "Entity.h"
#include <list>

class EntityManager : public Module
{
public:

	EntityManager();

	// Destructor
	virtual ~EntityManager();

	// Called before render is available
	bool Awake();

	// Called after Awake
	bool Start();

	// Called every frame
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Additional methods
	Entity* CreateEntity(EntityType type);

	void DestroyEntity(Entity* entity);

	void AddEntity(Entity* entity);

	//Control de dialogos
	void DialogoOn() { dialogo = true; }//parar player
	void DialogoOff() { dialogo = false; }//devolver control player

public:

	std::list<Entity*> entities;
	//Control de dialogos
	bool dialogo = false;

private:
	bool isPendingToDelente = false;

};
