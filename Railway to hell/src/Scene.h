#pragma once

#include "Module.h"
#include "Player.h"
#include "Terrestre.h"
#include <vector>
#include "GuiControlButton.h"
#include "SDL2/SDL_ttf.h"
#include "Volador.h"
#include "Boss.h"
#include "Item.h"
#include "Doors.h"
#include "Levers.h"

struct SDL_Texture;
enum class SceneState
{
	INTRO_SCREEN,
	GAMEPLAY
};
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

	//Implement the Load function
	void LoadState();
	//Implement the Save function
	void SaveState();


	//parar al player cuando sale un dialogo
	void DialogoOn();
	void DialogoOff();

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

	std::vector<Terrestre*> enemyList;
	std::vector<Volador*> voladorList; 
	std::vector<Boss*> bossList;
	std::vector<Item*> itemList;
	std::vector<Doors*> doorList;
	std::vector<Levers*> leverList;

	std::vector<Terrestre*>& GetEnemyList() { return enemyList; }
	std::vector<Volador*>& GetVoladorList() { return voladorList; } 
	std::vector<Boss*>& GetBossList() { return bossList; }
	std::vector<Item*>& GetItemList() { return itemList; }
	std::vector<Doors*>& GetDoorList() { return doorList; }
	std::vector<Levers*>& GetLeverList() { return leverList; }

	//Evitar salto player
	bool IsSkippingFirstInput() const { return skipFirstInput; }
	void ResetSkipInput() { skipFirstInput = false; }

	//Mirar escena
	SceneState GetCurrentState() const;

	void SetCurrentState(SceneState state);

	//Carga de texturas
	SDL_Texture* introScreenTexture = nullptr;

	//camara para lucha boss
	bool BossBattle = false;
	struct Bosses {
		int id;
		float x;
		float y;
	};
	//lista de escenas con su id
	std::vector<Bosses> Bosses = {
	//escena a la que va, x (590) de la camara, y (415) de la camara
	{3, 1282.73, 1151.67}
	};
	void EntrarBoss();
	void SalirBoss();

private:
	SDL_Texture* mouseTileTex = nullptr;
	SDL_Texture* textTexture = nullptr;//textura letras
	SDL_Texture* Fondo = nullptr;//textura fondo letras
	SDL_Texture* shadowTexture = nullptr;//textura sombra !!!!No va!!!!
	std::string tilePosDebug = "[0,0]";
	bool once = false;
	bool skipFirstInput = false;
	
	//cambio escenas
	SceneState currentState;

	// L16: TODO 2: Declare a GUI Control Button 
	GuiControlButton* guiBt;

	Player* player;

	//declaración de la musica de fondo
	int musicaFondoId = -1;
};