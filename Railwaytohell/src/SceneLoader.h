#pragma once
#include "Module.h"
#include <vector>
#include "pugixml.hpp"
#include "Item.h"
#include "Doors.h"
#include "Levers.h"
#include "MosaicLevers.h"
#include "MosaicPiece.h"
#include "MosaicPuzzle.h"
#include "Elevators.h"
#include "Entity.h"
#include "Engine.h"
#include "EntityManager.h" 

class Terrestre;
class Scene;
class Engine;
class Entity;
class Pathfinding;
class EntityManager;
class Item;
class Caronte;
class Bufon;
class Volador;
class MosaicPiece;
class MosaicLever;
class MosaicPuzzle;
class SceneLoader 
{
public:
	SceneLoader();
	~SceneLoader();
	void LoadScene(int level, int x, int y,bool fade,bool bosscam); // Here you should perform both the Load of the next scene and the Unload beforehand.
	void DrawScene(int level, int x, int y);
	void SetCurrentScene(int level);
	int GetCurrentLevel() const { return currentScene; }
	void FadeIn(float speed);// Black fading
	void FadeOut(float speed, bool loadscene, int level = -1, int x = -1, int y = -1); // Disappearance of black // loadscene means if it should use drawcurrentscene from the scene or drawscene from the sceneloader
	int currentScene;
	
private:
	void LoadEnemiesItems(pugi::xml_node sceneNode, int scene = -1);
	void UnLoadEnemiesItems();	
	void VisibilityScene(int level);
	void SetupMosaicPuzzle();
	void CargaSegundoPlano(int level, int x, int y);
	void VideoPrimerPlano();
};

