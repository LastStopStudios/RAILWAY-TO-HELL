#include "EntityManager.h"
#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Log.h"
#include "Item.h"
#include "Terrestre.h"
#include "Volador.h"
#include "Boss.h"
#include "Caronte.h"
#include "Doors.h"
#include "Levers.h"
#include "Elevators.h"
#include "Projectiles.h"
#include "Explosivo.h"
#include "Checkpoints.h"
#include <vector>
#include <algorithm>
#include "MosaicPiece.h"
#include "MosaicLevers.h"
#include "MosaicPuzzle.h"
#include "Devil.h"

EntityManager::EntityManager() : Module()
{
	name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager()
{}

// Called before render is available
bool EntityManager::Awake()
{
	LOG("Loading Entity Manager");
	bool ret = true;

	//Iterates over the entities and calls the Awake
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Awake();
	}
	return ret;
}

bool EntityManager::Start() {

	bool ret = true; 
	dialogo = false;
	Ascensor = true;
	//Iterates over the entities and calls Start
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Start();
	}

	return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	bool ret = true;

	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->CleanUp();
	}
	entities.clear();
	return ret;
}

Entity* EntityManager::CreateEntity(EntityType type)
{
	Entity* entity = nullptr; 

	//Instantiate entity according to the type and add the new entity to the list of Entities
	switch (type)
	{
	case EntityType::PLAYER:
		entity = new Player();
		entity->renderPriority = 10;
		break;
	case EntityType::ITEM:
		entity = new Item();
		break;
	case EntityType::TERRESTRE:
		entity = new Terrestre();
		entity->renderPriority = 9;
		break;
	case EntityType::EXPLOSIVO:
		entity = new Explosivo();
		entity->renderPriority = 8;
		break;
	case EntityType::VOLADOR:
		entity = new Volador();
		entity->renderPriority = 7;
		break;
	case EntityType::BOSS:
		entity = new Boss();
		break;
	case EntityType::BUFON:
		entity = new Bufon();
		break;
	case EntityType::DEVIL:
		entity = new Devil();
		break;
	case EntityType::CARONTE:
		entity = new Caronte();
		break;
	case EntityType::DOORS:
		entity = new Doors();
		break;
	case EntityType::LEVER:
		entity = new Levers();
		break;
	case EntityType::PROJECTILE:
		entity = new Projectiles();
		break;
	case EntityType::MOSAIC_PIECE:
		entity = new MosaicPiece();
		break;
	case EntityType::MOSAIC_LEVER:
		entity = new MosaicLever();
		break;
	case EntityType::CHECKPOINT:
		entity = new Checkpoints();
		entity->renderPriority = 4;
		break;
	case EntityType::ELEVATORS:
		entity = new Elevators();
		entity->renderPriority = 5;
		break;
	default:
		break;
	}

	entities.push_back(entity);

	return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
	for (auto it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == entity) {
			(*it)->CleanUp();
			delete* it; // Free the allocated memory
			entities.erase(it); // Remove the entity from the list
			break; // Exit the loop after removing the entity
		}
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if ( entity != nullptr) entities.push_back(entity);
}
bool EntityManager::PreUpdate(){ 
	bool ret = true;

	// Create a separate list for entities to be removed after the loop
	std::vector<Entity*> entitiesToRemove;

	// First update all entities
	for (const auto& entity : entities)
	{
		if (entity->active == false) continue;

		// Use IsPendingToDelete to check if entity should be removed
		if (entity->IsPendingToDelete()) {
			// Mark for removal but don't destroy yet
			entitiesToRemove.push_back(entity);
			continue;
		}
		ret = entity->PreUpdate();
	}

	// After the loop, remove all entities marked for deletion
	for (auto entity : entitiesToRemove) {
		DestroyEntity(entity);
	}
	return ret;
}
bool EntityManager::Update(float dt)
{
	bool ret = true;

	// Create a sorted copy of the entities for update and render
	std::vector<Entity*> sortedEntities(entities.begin(), entities.end());

	// Sort by render priority
	std::sort(sortedEntities.begin(), sortedEntities.end(),
		[](Entity* a, Entity* b) {
			return a->renderPriority < b->renderPriority;
		});

	// Process the entities in the correct order
	std::vector<Entity*> entitiesToRemove;

	for (const auto& entity : sortedEntities)
	{
		if (entity->active == false) continue;

		if (entity->IsPendingToDelete()) {
			entitiesToRemove.push_back(entity);
			continue;
		}

		ret = entity->Update(dt);
	}

	// Delete entities marked for removal
	for (auto entity : entitiesToRemove) {
		DestroyEntity(entity);
	}

	return ret;
}

bool EntityManager::PostUpdate() {
	bool ret = true;

	// Create a separate list for entities to be removed after the loop
	std::vector<Entity*> entitiesToRemove;

	// First update all entities
	for (const auto& entity : entities)
	{
		if (entity->active == false) continue;

		// Use IsPendingToDelete to check if entity should be removed
		if (entity->IsPendingToDelete()) {
			// Mark for removal but don't destroy yet
			entitiesToRemove.push_back(entity);
			continue;
		}
		ret = entity->PostUpdate();
	}

	// After the loop, remove all entities marked for deletion
	for (auto entity : entitiesToRemove) {
		DestroyEntity(entity);
	}

	return ret;
}
//Control de dialogos
void EntityManager :: DialogoOn() { dialogo = true; }//stop entities at the start of the dialogue
void EntityManager :: DialogoOff() { dialogo = false; }//return movement to the entities
//Elevator sensor control
void EntityManager::AscensorOn() { Ascensor = true; }//Open animation
void EntityManager::AscensorOff() { Ascensor = false; }//Block open animation
