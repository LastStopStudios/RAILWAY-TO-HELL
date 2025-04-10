#pragma once
#include "Module.h"
#include <vector>
#include "pugixml.hpp"
#include "Item.h"
#include "Doors.h"
#include "Levers.h"

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
	void LoadScene(int level, int x, int y,bool fade,bool bosscam); // Aqui dentro se tendría que hacer tanto el Load de la siguiente escena como previamente el Unload
	void SetCurrentScene(int level);
	int GetCurrentLevel() const { return currentScene; }

	int currentScene;
	std::vector<Item*> itemsList; 

private:
	void LoadEnemiesItems(pugi::xml_node sceneNode);
	void UnLoadEnemiesItems();
	void FadeOut(float speed); // Aparición del negro
	void FadeIn(float speed);// Desvanecimiento del negro
};

