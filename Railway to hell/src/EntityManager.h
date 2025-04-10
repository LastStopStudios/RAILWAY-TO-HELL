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

	//Dialog control
	void DialogoOn();//stop entities at the start of the dialogue
	void DialogoOff();//return movement to the entities

public:

	std::list<Entity*> entities;
	//Dialog control
	bool dialogo;

private:
	bool isPendingToDelente = false;

};
