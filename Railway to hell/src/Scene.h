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
#include "Caronte.h"
#include "Doors.h"
#include "Levers.h"

struct SDL_Texture;
enum class SceneState
{
	INTRO_SCREEN,
	TEXT_SCREEN,
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


	//stop entities when a dialog is displayed
	void DialogoOn();
	void DialogoOff();

	//Block scene change when fighting with a boss
	void BloquearSensor();
	void DesbloquearSensor();

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
	std::vector<Caronte*> caronteList;
	std::vector<Doors*> doorList;
	std::vector<Levers*> leverList;

	std::vector<Terrestre*>& GetEnemyList() { return enemyList; }
	std::vector<Volador*>& GetVoladorList() { return voladorList; } 
	std::vector<Boss*>& GetBossList() { return bossList; }
	std::vector<Item*>& GetItemList() { return itemList; }
	std::vector<Caronte*>& GetCaronteList() { return caronteList; }
	std::vector<Doors*>& GetDoorsList() { return doorList; }
	std::vector<Levers*>& GetLeversList() { return leverList; }

	//Avoid player jumping
	bool IsSkippingFirstInput() const { return skipFirstInput; }
	void ResetSkipInput() { skipFirstInput = false; }

	//Check out the current scene
	SceneState GetCurrentState() const;

	void SetCurrentState(SceneState state);

	//Loading textures
	SDL_Texture* introScreenTexture = nullptr;
	SDL_Texture* introTextoTexture = nullptr;
	float introTimeElapsed;
	//boss fight camera
	bool BossBattle = false;
	struct Bosses {
		int id;
		float x;
		float y;
	};
	//list of boss fights with their id
	std::vector<Bosses> Bosses = {
	//scene to which it goes, x (590) from camera, y (415) from camera
	{3, 1282.73, 1151.67}
	};
	//camera control
	void EntrarBoss();
	void SalirBoss();

private:
	SDL_Texture* mouseTileTex = nullptr;
	std::string tilePosDebug = "[0,0]";
	bool once = false;
	bool skipFirstInput = false;
	std::string currentMusic = "";
	bool introMusicPlaying = false;
	bool textMusicPlaying = false;
	//change scenes
	SceneState currentState;

	//Declare a GUI Control Button 
	GuiControlButton* guiBt;

	Player* player;

	//background music statement
	int musicaFondoId = -1;

	int introfx, textFx;

	std::vector<int> fx;
	std::vector<int> music;
	
public:
	void AddToMusic(int soundID) { music.push_back(soundID); }
	pugi::xml_node itemConfigNode; //mover a private

};