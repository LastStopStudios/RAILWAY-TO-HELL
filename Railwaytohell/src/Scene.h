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
#include "Estatua.h"
#include "Levers.h"
#include "Elevators.h"
#include "Projectiles.h"
#include "Bufon.h"
#include "MosaicLevers.h"
#include "MosaicPiece.h"
#include "MosaicPuzzle.h"
#include "Checkpoints.h"
#include "Devil.h"
#include "Spears.h"

struct SDL_Texture;
enum class SceneState
{
	INTRO_SCREEN,
	TEXT_SCREEN,
	GAMEPLAY,
	PAUSE_MENU,
	SETTINGS_MENU,
	CREDITS_MENU,
	EXIT_MENU,
	BACKTOTITTLE_MENU
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

	void DisableMenuButtons(); 
	void EnableMenuButtons();

	void DisablePauseButtons();
	void EnablePauseButtons();

	bool IsPaused() const { return pauseMenuOn; }
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
	std::vector<Spears*> spearsList;
	std::vector<Levers*> leverList;
	std::vector<Elevators*> elevatorList;
	std::vector<Estatua*> estatuaList;
	std::vector<Bufon*> bufonList;
	std::vector<Devil*> devilList;
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
	std::vector<Estatua*>& GetEstatuaList() { return estatuaList; }
	std::vector<Bufon*>& GetBufonList() { return bufonList; }
	std::vector<MosaicLever*>& GetMosaicLeversList() { return mosaicLeversList; }
	std::vector<MosaicPiece*>& GetMosaicPiecesList() { return mosaicPiecesList; }
	std::vector<MosaicPuzzle*>& GetMosaicPuzzleList() { return mosaicPuzzleList; }
	std::vector<Checkpoints*>& GetCheckpointsList() { return checkpointList; }
	std::vector<Devil*>& GetDevilList() { return devilList; }
	std::vector<Spears*>& GetSpearsList() { return spearsList; }

	//Avoid player jumping
	bool IsSkippingFirstInput() const { return skipFirstInput; }
	void ResetSkipInput() { skipFirstInput = false; }

	//Check out the current scene
	SceneState GetCurrentState() const;

	void SetCurrentState(SceneState state);

	//Loading textures
	SDL_Texture* introScreenTexture = nullptr;
	SDL_Texture* introTextoTexture = nullptr;
	SDL_Texture* settingsTexture = nullptr;
	SDL_Texture* creditsTexture = nullptr;
	SDL_Texture* pauseTexture = nullptr;

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
		{3, 1270, 1195, 1270, 1710.0f},
		{ 5, 1344, 1170, 1270, 1710.0f },
		{ 12, 153, 1128, 193, 616 }
	};
	//camera control
	void EntrarBoss();
	void SalirBoss();

	bool isBufonFight = false;
	bool isNomaFight = false;
	bool isDevilFight = false;

private:
	float targetCameraY = 0.0f;        // Y objetivo de la cámara
	float currentCameraY = 0.0f;       // Y actual de la cámara (para interpolación)
	float cameraTransitionSpeed = 5.0f; // Velocidad de transición (ajustable)
	bool isTransitioning = false;       // Flag para saber si estamos en transición

	SDL_Texture* mouseTileTex = nullptr;
	std::string tilePosDebug = "[0,0]";
	bool once = false;
	bool skipFirstInput = false;
	std::string currentMusic = "";
	bool introMusicPlaying = false;
	bool textMusicPlaying = false;
	//change scenes
	SceneState currentState;

	//Declare GUI Control Buttons 
	GuiControlButton* NewGame;
	GuiControlButton* Continue;
	GuiControlButton* Settings;
	GuiControlButton* Credits;
	GuiControlButton* ExitGame;

	//Pause Menu Buttons
	GuiControlButton* ResumeGame;
	GuiControlButton* BackToTitle;
	GuiControlButton* SettingsPause;
	GuiControlButton* ExitGamePause;

	// Buttons Texture
	SDL_Texture* NewGameNormal = nullptr;
	SDL_Texture* NewGameFocused = nullptr;
	SDL_Texture* NewGamePressed = nullptr;
	SDL_Texture* NewGameDOff = nullptr;

	SDL_Texture* ContinueNormal = nullptr;
	SDL_Texture* ContinueFocused = nullptr;
	SDL_Texture* ContinuePressed = nullptr;
	SDL_Texture* ContinueOff = nullptr;

	SDL_Texture* SettingsNormal = nullptr;
	SDL_Texture* SettingsFocused = nullptr;
	SDL_Texture* SettingsPressed = nullptr;
	SDL_Texture* SettingsOff = nullptr;

	SDL_Texture* CreditsNormal = nullptr;
	SDL_Texture* CreditsFocused = nullptr;
	SDL_Texture* CreditsPressed = nullptr;
	SDL_Texture* CreditsOff = nullptr;

	SDL_Texture* ExitNormal = nullptr;
	SDL_Texture* ExitFocused = nullptr;
	SDL_Texture* ExitPressed = nullptr;
	SDL_Texture* ExitOff = nullptr;

	SDL_Rect NewGamePos = { 520, 300, 120,20 };
	SDL_Rect ContinuePos = { 520, 350, 120,20 };
	SDL_Rect SettingsPos = { 520, 400, 120,20 };
	SDL_Rect CreditsPos = { 520, 450, 120,20 };
	SDL_Rect ExitGamePos = { 520, 500, 120,20 };

	bool hasStartedGame = false;
	bool exitRequested = false;

	bool pauseMenuOn = false;

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
	pugi::xml_node ballItemConfigNode;
	pugi::xml_node ballConfigNode;
	pugi::xml_node bigProjectileConfigNode;
	pugi::xml_node normalProjectileConfigNode;
	bool dibujar;

};