#pragma once

#include "Input.h"
#include "Render.h"

enum class EntityType
{
	PLAYER,
	ITEM,
	TERRESTRE,
	EXPLOSIVO,
	VOLADOR,
	BOSS,
	CARONTE,
	DOORS,
	LEVER,
	ELEVATORS,
	PROJECTILE,
	BUFON,
	MOSAIC_PIECE,
	MOSAIC_PUZZLE,
	MOSAIC_LEVER,
	CHECKPOINT,
	ESTATUA,
	DEVIL,
	SPEAR,
	UNKNOWN
};

class PhysBody;

class Entity
{
public:

	Entity(EntityType type) : type(type), active(true) {}

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool PreUpdate()
	{
		return true;
	}

	virtual bool Update(float dt)
	{
		return true;
	}

	virtual bool PostUpdate() 
	{ 
		return true; 
	}

	virtual bool CleanUp()
	{
		return true;
	}

	void Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();
		}
	}
	
	virtual bool IsPendingToDelete() const {
		return pendingToDelete;
	}

	virtual void OnCollision(PhysBody* physA, PhysBody* physB) {

	};

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

	};

public:
	bool pendingToDelete = false;
	std::string name;
	EntityType type;
	bool active = true;
	bool dialogo = true;
	// Possible properties, it depends on how generic we
	// want our Entity class, maybe it's not renderable...
	Vector2D position;       
	bool renderable = true;
	int renderPriority = 0;
};