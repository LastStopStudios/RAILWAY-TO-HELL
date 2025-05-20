#pragma once

#include "Module.h"
#include "Player.h"
#include "Terrestre.h"
#include "Explosivo.h"
#include <vector>
#include "GuiControlButton.h"
#include "SDL2/SDL_ttf.h"
#include "Volador.h"
#include "Boss.h"
#include "Item.h"
#include "Caronte.h"
#include "Doors.h"
#include "Levers.h"
#include "Elevators.h"
#include "Projectiles.h"
#include "Bufon.h"
#include "MosaicLevers.h"
#include "MosaicPiece.h"
#include "MosaicPuzzle.h"
#include "Checkpoints.h"


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

	//hitPlayer
	void hitearPlayer();

	// Handles multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control);

	// Draw the current scene
	void DrawCurrentScene();
	//Open puzzles doors
	bool OpenDoors() { return OpenDoor; }
	void SetOpenDoors() {  OpenDoor = true; }

public:
	//Open puzzle doors
	bool OpenDoor = false;

	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;

	}
	Player* GetPlayer() {
		return player;
	}

	std::vector<Terrestre*> enemyList;
	std::vector<Explosivo*> explosivoList;
	std::vector<Volador*> voladorList; 
	std::vector<Boss*> bossList;
	std::vector<Item*> itemList;
	std::vector<Caronte*> caronteList;
	std::vector<Doors*> doorList;
	std::vector<Levers*> leverList;
	std::vector<Elevators*> elevatorList;
	std::vector<Bufon*> bufonList;
	std::vector<MosaicLever*> mosaicLeversList;
	std::vector<MosaicPiece*> mosaicPiecesList;
	std::vector<MosaicPuzzle*> mosaicPuzzleList;
	std::vector<Checkpoints*> checkpointList;

	std::vector<Terrestre*>& GetEnemyList() { return enemyList; }
	std::vector<Explosivo*>& GetExploList() { return explosivoList; }
	std::vector<Volador*>& GetVoladorList() { return voladorList; } 
	std::vector<Boss*>& GetBossList() { return bossList; }
	std::vector<Item*>& GetItemList() { return itemList; }
	std::vector<Caronte*>& GetCaronteList() { return caronteList; }
	std::vector<Doors*>& GetDoorsList() { return doorList; }
	std::vector<Levers*>& GetLeversList() { return leverList; }
	std::vector<Elevators*>& GetElevatorsList() { return elevatorList; }
	std::vector<Bufon*>& GetBufonList() { return bufonList; }
	std::vector<MosaicLever*>& GetMosaicLeversList() { return mosaicLeversList; }
	std::vector<MosaicPiece*>& GetMosaicPiecesList() { return mosaicPiecesList; }
	std::vector<MosaicPuzzle*>& GetMosaicPuzzleList() { return mosaicPuzzleList; }
	std::vector<Checkpoints*>& GetCheckpointsList() { return checkpointList; }

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
		// limites eje x cam
		float leftBoundary;   // limite izq cam
		float rightBoundary;  // limite derecho cam
	};
	//list of boss fights with their id
	std::vector<Bosses> Bosses = {
		//scene to which it goes, x from camera, y from camera, leftBoundary, rightBoundary
		{3, 1270, 1195, 1270, 1710.0f}
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
	Boss* boss;
	//background music statement
	int musicaFondoId = -1;

	int introfx, textFx;

	std::vector<int> fx;
	std::vector<int> music;
	
public:
	void AddToMusic(int soundID) { music.push_back(soundID); }
	pugi::xml_node itemConfigNode; //mover a private
	pugi::xml_node whipItemConfigNode;
	pugi::xml_node doorItemConfigNode;
	pugi::xml_node ballConfigNode;
	bool dibujar;

};