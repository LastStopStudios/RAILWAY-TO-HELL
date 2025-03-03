#pragma once
#include "Module.h"
class Player; 

class SceneLoader 
{
public:
	SceneLoader();
	~SceneLoader();
	void LoadScene(int level); // Aqui dentro se tendría que hacer tanto el Load de la siguiente escena como previamente el Unload
	void SetCurrentScene(int level);
	int GetCurrentLevel() const { return currentScene; }
	int currentScene;
private:

};

