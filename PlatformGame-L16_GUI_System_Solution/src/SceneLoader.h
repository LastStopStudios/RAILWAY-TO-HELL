#pragma once
#include "Module.h"
class Player; 

class SceneLoader 
{
public:
	void LoadScene(int level); // Aqui dentro se tendría que hacer tanto el Load de la siguiente escena como previamente el Unload
	void UnLoadScene(int level); // Para clarificar el codigo lo separamos en dos funciones, pero UnLoad se llamará dentro de Load
	void SetCurrentScene(int level);
	int GetCurrentLevel() const { return currentScene; }
	int currentScene;
private:
	Player* player; 
	
};

