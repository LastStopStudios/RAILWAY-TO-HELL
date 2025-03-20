#pragma once
#include "Module.h"
#include <vector>
#include "pugixml.hpp"
#include "Item.h"
class Enemy;
class Scene;
class Engine;
class Entity;
class Pathfinding;
class EntityManager;
class Item;
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
	void LoadScene(int level); // Aqui dentro se tendría que hacer tanto el Load de la siguiente escena como previamente el Unload
	void SetCurrentScene(int level);
	int GetCurrentScene() const { return currentScene; }
	int currentScene;
	std::vector<Enemy*> enemysList;
	std::vector<Item*> itemsList; 
private:
	void LoadEnemiesItems(pugi::xml_node sceneNode);
	void UnLoadEnemiesItems();
	void FadeOut(float speed); // Aparición del negro
	void FadeIn(float speed);// Desvanecimiento del negro
};

