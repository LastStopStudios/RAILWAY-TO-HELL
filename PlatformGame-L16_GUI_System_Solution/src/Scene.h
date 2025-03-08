#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include "GuiControlButton.h"
#include "SDL2/SDL_ttf.h"


struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Return the player position
	Vector2D GetPlayerPosition();

	//L15 TODO 1: Implement the Load function
	void LoadState();
	//L15 TODO 2: Implement the Save function
	void SaveState();

	// Handles multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control);

public:
	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;

	}
	Player* GetPlayer() {
		return player;
	}


private:
	SDL_Texture* mouseTileTex = nullptr;
	SDL_Texture* textTexture = nullptr;
	SDL_Texture* shadowTexture = nullptr;
	std::string tilePosDebug = "[0,0]";
	bool once = false;

	//L03: TODO 3b: Declare a Player attribute
	std::vector<Enemy*> enemyList;

	// L16: TODO 2: Declare a GUI Control Button 
	GuiControlButton* guiBt;

	Player* player;

	void GenerateTextTexture();
	void UpdateTextAnimation(float dt);

	// Variables para manejar el texto
	bool showText = false;
	std::string displayText = "Guerra. La guerra nunca cambia. Cuando el fuego atómico consumió la tierra, aquellos que sobrevivieron lo hicieron en grandes bóvedas subterráneas. Cuando se abrieron, sus habitantes se trasladaron a través de las ruinas del viejo mundo para construir nuevas sociedades, establecer nuevas aldeas, formar tribus. A medida que pasaban las décadas, lo que había sido el suroeste de Estados Unidos se unió bajo la bandera de la Nueva República de California, dedicada a los valores del viejo mundo de la democracia y el estado de derecho. "; // texto que mostrar
	std::string currentText = ""; // Texto que se muestra
	int textIndex = 0;           // Índice del último carácter mostrado
	float textTimer = 0.0f;      // Temporizador para controlar la velocidad
	float textSpeed = 5.0f;     // Velocidad de escritura (segundos entre letras)
	int textMaxWidth = 900; // Máximo ancho antes de saltar de línea

};