#pragma once
#include "Module.h"
#include <vector>
#include "pugixml.hpp"
#include "Item.h"
#include "Doors.h"
#include "Levers.h"
#include "Elevators.h"

class Terrestre;
class Scene;
class Engine;
class Entity;
class Pathfinding;
class EntityManager;
class Item;
class Caronte;
#include "Entity.h"
//#include "Scene.h"
//#include "Enemy.h"
#include "Engine.h"
//#include "Pathfinding.h" 
#include "EntityManager.h" 

class SceneLoader 
{
public:
	SceneLoader();
	~SceneLoader();
	void LoadScene(int level, int x, int y,bool fade,bool bosscam); // Here you should perform both the Load of the next scene and the Unload beforehand.
	void SetCurrentScene(int level);
	int GetCurrentLevel() const { return currentScene; }
	void FadeIn(float speed);// Black fading
	int currentScene;

private:
	void LoadEnemiesItems(pugi::xml_node sceneNode);
	void UnLoadEnemiesItems();
	void FadeOut(float speed); // Appearance of black
	
};

