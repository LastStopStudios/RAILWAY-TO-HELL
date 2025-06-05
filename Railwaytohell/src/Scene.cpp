#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Terrestre.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "SDL2/SDL_ttf.h"
#include "DialogoM.h"
#include "Volador.h"
#include "Elevators.h"
#include "Explosivo.h"
#include "Ffmpeg.h"
#include "GlobalSettings.h"
#include "Devil.h"
#include "Spears.h"
#include "Mapa.h"

Scene::Scene() : Module()
{
	name = "scene";
	currentState = SceneState::INTRO_SCREEN;
	previousState = SceneState::INTRO_SCREEN;  
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;
	morido = false;
	//Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
	
	//Create a new item using the entity manager and set the position to (200, 672) to test
	for(pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
	{
		std::string type = itemNode.attribute("name").as_string();

		if (type == "Door key"){
			doorItemConfigNode = itemNode;
		}
		else if (type == "Whip") {
			whipItemConfigNode = itemNode;
		}
		else if (type == "Ball") {
			ballItemConfigNode = itemNode;
		}
		else {
		Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
		item->SetParameters(itemNode);
		//item->SetAliveInXML();
		itemList.push_back(item);
		}
	}

	for (pugi::xml_node checkpointNode = configParameters.child("entities").child("checkpoints").child("checkpoint"); checkpointNode; checkpointNode = checkpointNode.next_sibling("checkpoint"))
	{
		std::string type = checkpointNode.attribute("name").as_string(); 

		if (type == "beta") {
			//checkpointBetaConfigNode = checkpointNode;
		}
		else if (type == "aplha") {
			//checkpointAlphaConfigNode = checkpointNode;
		}
		else {
			Checkpoints* checkpoint = (Checkpoints*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT);
			checkpoint->SetParameters(checkpointNode);
			checkpointList.push_back(checkpoint);
		}

	}

	for (pugi::xml_node projectileNode = configParameters.child("entities").child("projectiles").child("projectile"); projectileNode; projectileNode = projectileNode.next_sibling("projectile"))
	{
		std::string type = projectileNode.attribute("type").as_string();
		if (type == "big")	bigProjectileConfigNode = projectileNode;
		if (type == "normal") normalProjectileConfigNode = projectileNode;
		
	}
	for (pugi::xml_node spearNode = configParameters.child("entities").child("spear"); spearNode; spearNode = spearNode.next_sibling("spear"))
	{
		Spears* spear = (Spears*)Engine::GetInstance().entityManager->CreateEntity(EntityType::SPEAR);
		spear->SetParameters(spearNode);
		spearsList.push_back(spear);
	}

	for (pugi::xml_node doorNode = configParameters.child("entities").child("doors").child("door"); doorNode; doorNode = doorNode.next_sibling("door"))
	{
		Doors* door = (Doors*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DOORS);
		door->SetParameters(doorNode);
		doorList.push_back(door);
	}

	for (pugi::xml_node leverNode = configParameters.child("entities").child("levers").child("lever"); leverNode; leverNode = leverNode.next_sibling("lever"))
	{
		Levers* lever = (Levers*)Engine::GetInstance().entityManager->CreateEntity(EntityType::LEVER);
		lever->SetParameters(leverNode);
		leverList.push_back(lever);
	}

	for (pugi::xml_node elevatorNode = configParameters.child("entities").child("elevators").child("elevator"); elevatorNode; elevatorNode = elevatorNode.next_sibling("elevator"))
	{
		Elevators* elevator = (Elevators*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ELEVATORS);
		elevator->SetParameters(elevatorNode);
		elevatorList.push_back(elevator);
	}

	for (pugi::xml_node estatuaNode = configParameters.child("entities").child("estatuas").child("estatua"); estatuaNode; estatuaNode = estatuaNode.next_sibling("estatua"))
	{
		Estatua* estatua = (Estatua*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ESTATUA);
		estatua->SetParameters(estatuaNode);
		estatuaList.push_back(estatua);
	}

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		std::string type = enemyNode.attribute("type").as_string();
		std::string ref = enemyNode.attribute("ref").as_string();
		if (type == "rastrero") {
			Terrestre* enemy = (Terrestre*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TERRESTRE);
			enemy->SetParameters(enemyNode);
			enemyList.push_back(enemy);
		}
		if (type == "amego") {
			Explosivo* explo = (Explosivo*)Engine::GetInstance().entityManager->CreateEntity(EntityType::EXPLOSIVO);
			explo->SetParameters(enemyNode);
			explosivoList.push_back(explo);
		}
		if (type == "volador") {
			Volador* volador = (Volador*)Engine::GetInstance().entityManager->CreateEntity(EntityType::VOLADOR);
			volador->SetParameters(enemyNode);
			voladorList.push_back(volador);  
		}
		if (type == "boss" && ref == "noma") {
			Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
			boss->SetParameters(enemyNode);
			bossList.push_back(boss);
		}
		if (type == "boss" && ref == "bufon") {
			Bufon* bufon = (Bufon*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BUFON);
			bufon->SetParameters(enemyNode);
			bufonList.push_back(bufon);
		}
		if (type == "boss" && ref == "devil") {
			Devil* devil = (Devil*)Engine::GetInstance().entityManager->CreateEntity(EntityType::DEVIL);
			devil->SetParameters(enemyNode);
			devilList.push_back(devil);
		}
		if (type == "guardian") {
			Caronte* caronte = (Caronte*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARONTE);
			caronte->SetParameters(enemyNode);
			caronteList.push_back(caronte);
		}
	}

	

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//Cargar Texturas splash screen
	introScreenTexture = Engine::GetInstance().textures->Load("Assets/Textures/Intro_Screen.png");
	introTextoTexture = Engine::GetInstance().textures->Load("Assets/Textures/IntroTexto2.png"); //png reescale to 853 X 512 to work with camera zoom
	settingsTexture = Engine::GetInstance().textures->Load("Assets/Textures/Settings_Background.png");
	creditsTexture = Engine::GetInstance().textures->Load("Assets/Textures/Credits_Background.png");
	pauseTexture = Engine::GetInstance().textures->Load("Assets/Textures/Pause_Background.png");
	controlsTexture1 = Engine::GetInstance().textures->Load("Assets/Textures/controls1.png");
	controlsTexture2 = Engine::GetInstance().textures->Load("Assets/Textures/controls2.png");
	controlsTexture3 = Engine::GetInstance().textures->Load("Assets/Textures/controls3.png");
	settingsPauseMenu = Engine::GetInstance().textures->Load("Assets/Textures/Settings_PauseMenu.png");

	//Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());

	// Texture to highligh mouse position 
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Maps/MapMetadata.png");
	std::string introMusicPath = "Assets/Audio/Music/Intro.ogg";
	std::string textMusicPath = "Assets/Audio/Music/Text.ogg";
	/*Engine::GetInstance().audio.get()->SetFxVolume(introfx, 1000);
	Engine::GetInstance().audio.get()->SetFxVolume(textFx, 5);*/
	// Initalize the camera position
	int w, h;
	Engine::GetInstance().window.get()->GetWindowSize(w, h);
	Engine::GetInstance().render.get()->camera.x = 0;
	Engine::GetInstance().render.get()->camera.y = 0;

	Engine::GetInstance().ffmpeg->Awake();
	Engine::GetInstance().ffmpeg->Start(); 

	// Instantiate a new GuiControlButton in the Scene
// Button
	SDL_Rect NewGamePos = { 580, 350, 120,20 };
	SDL_Rect ContinuePos = { 580, 400, 120,20 };
	SDL_Rect ControlsMainMenuPos = { 580, 450, 120,20 };
	SDL_Rect SettingsPos = { 580, 500, 120,20 };
	SDL_Rect CreditsPos = { 580, 550, 120,20 };
	SDL_Rect ExitGamePos = { 580, 600, 120,20 };

	NewGameNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/NewGameNormal.png");
	NewGameFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/NewGameFocused.png");
	NewGamePressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/NewGamePressed.png");
	NewGameDOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/NewGameNormal.png");

	ContinueNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ContinueNormal.png");
	ContinueFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ContinueFocused.png");
	ContinuePressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ContinuePressed.png");
	ContinueOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ContinueNormalOff.png");

	SettingsNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/SettingsNormal.png");
	SettingsFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/SettingsFocused.png");
	SettingsPressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/SettingsPressed.png");
	SettingsOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/SettingsNormal.png");

	CreditsNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/CreditsNormal.png");
	CreditsFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/CreditsFocused.png");
	CreditsPressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/CreditsPressed.png");
	CreditsOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/CreditsNormal.png");

	ExitNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ExitNormal.png");
	ExitFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ExitFocused.png");
	ExitPressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ExitPressed.png");
	ExitOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/ExitNormal.png");

	// Pause Menu Buttons

	ResumeNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ResumeNormal.png");
	ResumeFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ResumeFocused.png");
	ResumePressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ResumePressed.png");
	ResumeOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ResumeNormal.png");

	ControlsNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ControlsNormal.png");
	ControlsFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ControlsFocused.png");
	ControlsPressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ControlsPressed.png");
	ControlsOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/ControlsNormal.png");

	BackToTitleNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/BackToTitleNormal.png");
	BackToTitleFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/BackToTitleFocused.png");
	BackToTitlePressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/BackToTitlePressed.png");
	BackToTitleOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/PauseMenu/BackToTitleNormal.png");

	FullScreenNormal = Engine::GetInstance().textures->Load("Assets/Textures/GUI/Box.png");
	FullScreenFocused = Engine::GetInstance().textures->Load("Assets/Textures/GUI/Tick.png");
	FullScreenPressed = Engine::GetInstance().textures->Load("Assets/Textures/GUI/Tick.png");
	FullScreenOff = Engine::GetInstance().textures->Load("Assets/Textures/GUI/Box.png");

	sliderBase = Engine::GetInstance().textures->Load("Assets/Textures/GUI/SliderBase.png");
	sliderHandle = Engine::GetInstance().textures->Load("Assets/Textures/GUI/SliderHandle.png");

	// Create the menu buttons
	NewGame = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 2, " ", NewGamePos, this);
	NewGame->SetTextures(NewGameNormal, NewGameFocused, NewGamePressed, NewGameDOff);

	Continue = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 3, " ", ContinuePos, this);
	Continue->SetTextures(ContinueNormal, ContinueFocused, ContinuePressed, ContinueOff);
	Continue->SetState(GuiControlState::OFF);

	ControlsMainMenu = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 13, " ", ControlsMainMenuPos, this);
	ControlsMainMenu->SetTextures(ControlsNormal, ControlsFocused, ControlsPressed, ControlsOff);

	Settings = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 4, " ", SettingsPos, this);
	Settings->SetTextures(SettingsNormal, SettingsFocused, SettingsPressed, SettingsOff);

	Credits = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 5, " ", CreditsPos, this);
	Credits->SetTextures(CreditsNormal, CreditsFocused, CreditsPressed, CreditsOff);

	ExitGame = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 6, " ", ExitGamePos, this);
	ExitGame->SetTextures(ExitNormal, ExitFocused, ExitPressed, ExitOff);

	// Fullscreen Checkbox
	fullscreenState = Engine::GetInstance().window->IsFullscreen();

	SDL_Rect fullscreenPos = { 520, 400, 40, 40 };
	FullScreenCheckbox = (CheckBox*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::CHECKBOX, 11, " ", fullscreenPos, this);
	FullScreenCheckbox->SetTextures(FullScreenNormal, FullScreenPressed);
	FullScreenCheckbox->SetState(GuiControlState::DISABLED);


	// Music Slider
	SDL_Rect musicSliderPos = { 580, 270, 200, 10 }; 
	musicSlider = (GuiControlSlider*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 14, " ", musicSliderPos, this, { 0, 128 });// min=0, max=128

	musicSlider->SetTextures(sliderBase, sliderHandle);
	musicSlider->SetHandleSize(20, 20); // Handle size
	int currentVolume = Engine::GetInstance().audio->GetMusicVolume();
	musicSlider->SetValue(currentVolume);
	Engine::GetInstance().audio->SetMusicVolume(currentVolume);
	musicSlider->UpdateHandlePosition();
	musicSlider->SetState(GuiControlState::DISABLED);

	// SFX Slider
	SDL_Rect sfxSliderPos = { 580, 310, 200, 10 };
	fxSlider = (GuiControlSlider*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 15, " ", sfxSliderPos, this, { 0, 128 });// min=0, max=128
	fxSlider->SetTextures(sliderBase, sliderHandle);
	fxSlider->SetHandleSize(20, 20);
	fxSlider->SetValue(Engine::GetInstance().audio->GetFxVolume()); 
	fxSlider->UpdateHandlePosition();
	fxSlider->SetState(GuiControlState::DISABLED);


	//Draw player
	dibujar = false;

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	switch (currentState)
	{
	case SceneState::INTRO_SCREEN:

		if (hasStartedGame && Continue->state == GuiControlState::OFF) {
			Continue->SetState(GuiControlState::NORMAL);
		}

		if (!introMusicPlaying) {

			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Devil.ogg", 1.0f);
			Engine::GetInstance().audio.get()->SetMusicVolume(2);
			//Engine::GetInstance().sceneLoader->FadeOut(2.5f, false);// Animation speed (FadeOut)

			introMusicPlaying = true;
			textMusicPlaying = false;
			currentMusic = "intro";
		}
		if (introMusicPlaying) {
			DrawCurrentScene();
		}
		// Original keyboard input
		//if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
		//	Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
		//	SetCurrentState(SceneState::TEXT_SCREEN);
		//	skipFirstInput = true;
		//}
		//Check controller input
		if (Engine::GetInstance().IsControllerConnected()) {
			SDL_GameController* controller = Engine::GetInstance().GetGameController();
			// SDL_CONTROLLER_BUTTON_A corresponds to the bottom button (X on PlayStation, A on Xbox)
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
				Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
				SetCurrentState(SceneState::TEXT_SCREEN);
				skipFirstInput = true;
			}
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Q) == KEY_DOWN) {
			SetCurrentState(SceneState::GAMEPLAY);
		}
		dibujar = true;
		break;
	case SceneState::CONTROLS_MENU:
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
			SetCurrentState(previousState);
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN) {
			currentControlsPage--;
			if (currentControlsPage < 0) currentControlsPage = 2;
		}
		else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN) {
			currentControlsPage++;
			if (currentControlsPage > 2) currentControlsPage = 0;
		}

		DrawCurrentScene();
		break;
	case SceneState::SETTINGS_MENU:
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
			SetCurrentState(previousState);
		}
		DrawCurrentScene();

		break;
	case SceneState::CREDITS_MENU:
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
			SetCurrentState(SceneState::INTRO_SCREEN);
		}

		DrawCurrentScene();

		break;
	case SceneState::EXIT_MENU:
		break;
	case SceneState::BACKTOTITTLE_MENU:
		SetCurrentState(SceneState::INTRO_SCREEN);
		break;
	case SceneState::TEXT_SCREEN:
		Engine::GetInstance().ffmpeg->ConvertPixels("Assets/Videos/Escena1.mp4");

		SetCurrentState(SceneState::GAMEPLAY);
		// Original keyboard input
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
			Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
			SetCurrentState(SceneState::GAMEPLAY);
		}
		//Check controller input
		if (Engine::GetInstance().IsControllerConnected()) {
			SDL_GameController* controller = Engine::GetInstance().GetGameController();
			// SDL_CONTROLLER_BUTTON_A corresponds to the bottom button (X on PlayStation, A on Xbox)
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
				Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
				SetCurrentState(SceneState::GAMEPLAY);
			}
		}
		break;
	case SceneState::GAMEPLAY:

		pauseMenuOn = false;

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
			SetCurrentState(SceneState::PAUSE_MENU);
			pauseMenuOn = true;
		}

		if (!pauseMenuOn) {
			if (!musicOn) {
				Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 1.0f);
				musicOn = true;
			}
			for (auto puzzle : mosaicPuzzleList) {
				puzzle->Update(dt);
			}
			//Make the camera movement independent of framerate
			float camSpeed = 1;
			//Implement a method that repositions the player in the map with a mouse click

			//Get mouse position and obtain the map coordinate
			int scale = Engine::GetInstance().window.get()->GetScale();
			Vector2D mousePos = Engine::GetInstance().input.get()->GetMousePosition();
			Vector2D mouseTile = Engine::GetInstance().map.get()->WorldToMap(mousePos.getX() - Engine::GetInstance().render.get()->camera.x / scale,
				mousePos.getY() - Engine::GetInstance().render.get()->camera.y / scale);

			//Render a texture where the mouse is over to highlight the tile, use the texture 'mouseTileTex'
			Vector2D highlightTile = Engine::GetInstance().map.get()->MapToWorld(mouseTile.getX(), mouseTile.getY());
			SDL_Rect rect = { 0,0,32,32 };
			/*Engine::GetInstance().render.get()->DrawTexture(mouseTileTex,
				highlightTile.getX(),
				highlightTile.getY(),
				&rect);*/

				// saves the tile pos for debugging purposes
			if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
				tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + "," + std::to_string((int)mouseTile.getY()) + "] ";
				once = true;
			}
		}
		//If mouse button is pressed modify enemy position
		/*if (Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_DOWN) {
		enemyList[0]->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
		enemyList[0]->ResetPath();
		}*/
		if (morido == true) {
			if (Engine::GetInstance().mapa.get()->remember1 && Engine::GetInstance().mapa.get()->remember2 && Engine::GetInstance().mapa.get()->remember3 && Engine::GetInstance().mapa.get()->remember4 && Engine::GetInstance().mapa.get()->remember5 && Engine::GetInstance().mapa.get()->remember6 && Engine::GetInstance().mapa.get()->remember7 && Engine::GetInstance().mapa.get()->remember8) {
				Engine::GetInstance().ffmpeg->ConvertPixels("Assets/Videos/Final2.mp4");
				GlobalSettings::GetInstance().SetTextureMultiplier(1.5);
				Engine::GetInstance().scene->SetCurrentState(SceneState::INTRO_SCREEN);
			}
			else {
				Engine::GetInstance().ffmpeg->ConvertPixels("Assets/Videos/Final1.mp4");
				GlobalSettings::GetInstance().SetTextureMultiplier(1.5);
				Engine::GetInstance().scene->SetCurrentState(SceneState::INTRO_SCREEN);
			}
			morido = false;
		}
		break;
	case SceneState::PAUSE_MENU:
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
			SetCurrentState(SceneState::GAMEPLAY);
		}
		DrawCurrentScene();
		break;
	}


	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;
	SceneState currentState = GetCurrentState();

	if (currentState == SceneState::GAMEPLAY) {
		// Get current window dimensions
		int window_width, window_height;
		Engine::GetInstance().window.get()->GetWindowSize(window_width, window_height);
		int center_x = window_width / 2;
		int center_y = window_height / 2;

		// Logical base offsets (not affected by zoom)
		const int BASE_OFFSET_X = -50;
		const int BASE_OFFSET_Y = 86;

		if (BossBattle == false) {
			// Follow player position in world space (normal gameplay - no zoom check)
			float cameraBaseX = (player->position.getX() * -1.0f) + (center_x / GlobalSettings::GetInstance().GetTextureMultiplier()) + BASE_OFFSET_X;
			float cameraBaseY = (player->position.getY() * -1.0f) + (center_y / GlobalSettings::GetInstance().GetTextureMultiplier()) + BASE_OFFSET_Y;

			// Apply zoom only to the final result
			Engine::GetInstance().render.get()->camera.x = (int)(cameraBaseX * GlobalSettings::GetInstance().GetTextureMultiplier());
			Engine::GetInstance().render.get()->camera.y = (int)(cameraBaseY * GlobalSettings::GetInstance().GetTextureMultiplier());
		}
		else {
			// Boss battle camera behavior - check zoom level
			float currentZoom = GlobalSettings::GetInstance().GetTextureMultiplier();
			int currentLevel = Engine::GetInstance().sceneLoader->GetCurrentLevel();

			for (const auto& boss : Bosses) {
				if (boss.id == currentLevel) {

					// Check if zoom is different from default (1.5f)
					if (currentZoom != 1.5f) {
						// Different offsets for non-default zoom in boss battles
						const int BOSS_ZOOM_OFFSET_X = -20;  // Adjust as needed
						const int BOSS_ZOOM_Y_OFFSET = 40;  // Adjust as needed

						// Fixed Y position for boss battles with adjusted offset
						float cameraBaseY = (-boss.y) + BOSS_ZOOM_Y_OFFSET;
						Engine::GetInstance().render.get()->camera.y = (int)(cameraBaseY * currentZoom);

						// Follow player X position with constraints and adjusted offset
						float playerX = player->position.getX();
						float desiredCameraBaseX = (playerX * -1.0f) + (center_x / currentZoom) + BOSS_ZOOM_OFFSET_X;

						// Clamp camera position within boss area bounds
						float leftLimitBase = -boss.leftBoundary;
						float rightLimitBase = -boss.rightBoundary + 424;

						if (desiredCameraBaseX > leftLimitBase) {
							desiredCameraBaseX = leftLimitBase;
						}
						if (desiredCameraBaseX < rightLimitBase) {
							desiredCameraBaseX = rightLimitBase;
						}

						// Apply zoom to final X position
						Engine::GetInstance().render.get()->camera.x = (int)(desiredCameraBaseX * currentZoom);
					}
					else {
						// Default boss battle camera behavior for zoom 1.5f
						// Check if it's scene 12 for special Y offset
						int yOffset = (currentLevel == 12) ? -190 : -115; 

						// Fixed Y position for boss battles (world space)
						float cameraBaseY = (-boss.y) + yOffset;
						Engine::GetInstance().render.get()->camera.y = (int)(cameraBaseY * currentZoom);

						// Follow player X position with constraints (world space)
						float playerX = player->position.getX();
						float desiredCameraBaseX = (playerX * -1.0f) + (center_x / currentZoom) + BASE_OFFSET_X;

						// Clamp camera position within boss area bounds
						float leftLimitBase = -boss.leftBoundary;
						float rightLimitBase = -boss.rightBoundary;

						if (desiredCameraBaseX > leftLimitBase) {
							desiredCameraBaseX = leftLimitBase;
						}
						if (desiredCameraBaseX < rightLimitBase) {
							desiredCameraBaseX = rightLimitBase;
						}

						// Apply zoom to final X position
						Engine::GetInstance().render.get()->camera.x = (int)(desiredCameraBaseX * currentZoom);
					}
					break;
				}
			}
		}
	}



	if (exitRequested) {
		ret = false; // Exit the game
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Z) == KEY_DOWN) {
		ret = false;
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		LoadState(); // Load game state
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		SaveState(); // Save game state

	return ret;
}

void Scene::EntrarBoss() { BossBattle = true;
if (BossBattle) {
	if (isBufonFight) {
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Bufon.ogg", 10.0f);
		
	}
	else if (isNomaFight ) {
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Noma.ogg", 10.0f);
		
	}
	else if (isDevilFight)
	{
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Devil.ogg", 10.0f);

	}
}
}

void Scene::SalirBoss() { BossBattle = false;

Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 10.0f);
}

// Called before quitting
bool Scene::CleanUp()
{
	if (introScreenTexture != nullptr)
	{
		Engine::GetInstance().textures->UnLoad(introScreenTexture);
		//introScreenTexture = nullptr;
	}
	if (introTextoTexture != nullptr)
	{
		Engine::GetInstance().textures->UnLoad(introTextoTexture);
		//introTextoTexture = nullptr;
	}

	Engine::GetInstance().textures->UnLoad(NewGameNormal);
	Engine::GetInstance().textures->UnLoad(NewGameFocused);
	Engine::GetInstance().textures->UnLoad(NewGamePressed);
	Engine::GetInstance().textures->UnLoad(NewGameDOff);

	Engine::GetInstance().textures->UnLoad(ContinueNormal);
	Engine::GetInstance().textures->UnLoad(ContinueFocused);
	Engine::GetInstance().textures->UnLoad(ContinuePressed);
	Engine::GetInstance().textures->UnLoad(ContinueOff);

	Engine::GetInstance().textures->UnLoad(SettingsNormal);
	Engine::GetInstance().textures->UnLoad(SettingsFocused);
	Engine::GetInstance().textures->UnLoad(SettingsPressed);
	Engine::GetInstance().textures->UnLoad(SettingsOff);

	Engine::GetInstance().textures->UnLoad(CreditsNormal);
	Engine::GetInstance().textures->UnLoad(CreditsFocused);
	Engine::GetInstance().textures->UnLoad(CreditsPressed);
	Engine::GetInstance().textures->UnLoad(CreditsOff);

	Engine::GetInstance().textures->UnLoad(ExitNormal);
	Engine::GetInstance().textures->UnLoad(ExitFocused);
	Engine::GetInstance().textures->UnLoad(ExitPressed);
	Engine::GetInstance().textures->UnLoad(ExitOff);

	for (auto puzzle : mosaicPuzzleList) {
		puzzle->CleanUp();
		delete puzzle; 
	}
	mosaicPuzzleList.clear();

	player->ResetToInitPosition();

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return false;
	}
	// Reset lastCheckpointScene to 1 if it's not already 1
	pugi::xml_node gamestateNode = loadFile.child("config").child("gamestate");
	pugi::xml_node lastCheckpointNode = gamestateNode.child("lastCheckpointScene").child("scene");

	int currentValue = lastCheckpointNode.attribute("value").as_int();
	if (currentValue != 1) {
		lastCheckpointNode.attribute("value").set_value(1);
	}
	pugi::xml_node sceneNode;
	int maxScenes = 12;
	for (int i = 0; i < maxScenes; ++i) {
		if (i == 0) { sceneNode = loadFile.child("config").child("scene"); }
		else if (i == 1){ sceneNode = loadFile.child("config").child("scene2");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 2) { sceneNode = loadFile.child("config").child("scene3");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 3) { sceneNode = loadFile.child("config").child("scene4");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 4) { sceneNode = loadFile.child("config").child("scene5");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 5) { sceneNode = loadFile.child("config").child("scene6");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 6) { sceneNode = loadFile.child("config").child("scene7");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 7) { sceneNode = loadFile.child("config").child("scene8");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 8) { sceneNode = loadFile.child("config").child("scene9");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 9) { sceneNode = loadFile.child("config").child("scene10");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 10) { sceneNode = loadFile.child("config").child("scene11"); /*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 11) {
			sceneNode = loadFile.child("config").child("scene12"); /*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}

		//checkpoints
		pugi::xml_node checkpointsNode = sceneNode.child("entities").child("checkpoints");

		for (pugi::xml_node checkpointNode : checkpointsNode.children("checkpoint")) {
			checkpointNode.attribute("pendingToChangeAnim").set_value(false);
			checkpointNode.attribute("activated").set_value(false);
		}

		//item
		pugi::xml_node itemsNode = sceneNode.child("entities").child("items");

		for (pugi::xml_node itemNode : itemsNode.children("item")) {
			std::string name = itemNode.attribute("name").as_string();
			if (name == "Whip") {
				itemNode.attribute("created").set_value(false);
			}
			itemNode.attribute("death").set_value(0);
			itemNode.attribute("savedDeath").set_value(0);
		}

		//enemies
		pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");

		for (pugi::xml_node enemyNode : enemiesNode.children("enemy")) {
			std::string type = enemyNode.attribute("type").as_string();
			if (type == "boss") {
				if(i == 2){
				enemyNode.attribute("x").set_value(2045);
				enemyNode.attribute("y").set_value(1648);
				}

				enemyNode.attribute("death").set_value(0);
				enemyNode.attribute("savedDeath").set_value(0);
			}
		}
	}

	pugi::xml_node windowNode = loadFile.child("config").child("window");
	if (windowNode) {
		pugi::xml_node fullscreenNode = windowNode.child("fullscreen_window");
		if (fullscreenNode) {
			fullscreenNode.attribute("value").set_value(false);
		}
	}

	loadFile.save_file("config.xml");

	//for (auto& item : itemList) {
	//	item->SetAliveInXML();
	//	item->SetSavedDeathToAliveInXML();
	//}

	//for (auto& boss : bossList) {

	//	boss->SetAliveInXML();
	//	boss->SetSavedDeathToAliveInXML();

	//}

	return true;
}

// Return the player position
Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}

// L15 TODO 1: Implement the Load function
void Scene::LoadState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}
	pugi::xml_node sceneNode;

	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();
	if (currentScene == 1) {
		sceneNode = loadFile.child("config").child("scene");
	}
	if (currentScene == 2) {
		sceneNode = loadFile.child("config").child("scene2");
	}
	if (currentScene == 3) {
		sceneNode = loadFile.child("config").child("scene3");
	}
	if (currentScene == 4) {
		sceneNode = loadFile.child("config").child("scene4");
	}
	if (currentScene == 5) {
		sceneNode = loadFile.child("config").child("scene5");
	}
	if (currentScene == 6) {
		sceneNode = loadFile.child("config").child("scene6");
	}
	if (currentScene == 7) {
		sceneNode = loadFile.child("config").child("scene7");
	}
	if (currentScene == 8) {
		sceneNode = loadFile.child("config").child("scene8");
	}
	if (currentScene == 9) {
		sceneNode = loadFile.child("config").child("scene9");
	}
	if (currentScene == 10) {
		sceneNode = loadFile.child("config").child("scene10");
	}
	if (currentScene == 11) {
		sceneNode = loadFile.child("config").child("scene11");
	}

	//Read XML and restore information

	//Player position
	Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int(),
		sceneNode.child("entities").child("player").attribute("y").as_int());
	player->SetPosition(playerPos);

	//enemies
	pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
	if (itemsNode) {
		// for (auto& enemy : enemyList) 
		for (pugi::xml_node itemNode : itemsNode.children("item")) {
			// read XML
			std::string xmlRef = itemNode.attribute("ref").as_string();
			int xmlDeath = itemNode.attribute("death").as_int();
			int xmlSavedDeath = itemNode.attribute("savedDeath").as_int();
			Vector2D pos(
				itemNode.attribute("x").as_int(),
				itemNode.attribute("y").as_int()
			);

			// case 1 update position if: death=0 & savedDeath=0 
			if (xmlDeath == 0 && xmlSavedDeath == 0) {
				for (int i = 0; i < itemList.size(); ++i) {
					if (itemList[i]->GetRef() == xmlRef) {
						itemList[i]->SetPosition(pos);
						//itemList[i]->SetAliveInXML();
						//itemList[i]->SetEnabled(true);;
					}
				}
			}
			// case 2 create enemy if: death=0 & savedDeath=1 
			else if (xmlDeath == 1 && xmlSavedDeath == 0) {
				for (int i = 0; i < itemList.size(); ++i) {
					if (itemList[i]->GetRef() == xmlRef) {
						itemList[i]->SetPosition(pos);
						itemList[i]->SetAliveInXML();
						itemList[i]->SetEnabled(true);;
					}
				}
			}

			// case 3 do nothing if: death=1 & savedDeath=1
		}
	}

	//enemies
	pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
	if (enemiesNode) {
		// for (auto& enemy : enemyList) 
		for (pugi::xml_node enemyNode : enemiesNode.children("enemy")) {
			// read XML
			std::string type = enemyNode.attribute("type").as_string();
			if (type == "boss") {
				std::string xmlRef = enemyNode.attribute("ref").as_string();
				int xmlDeath = enemyNode.attribute("death").as_int();
				int xmlSavedDeath = enemyNode.attribute("savedDeath").as_int();
				Vector2D pos(
					enemyNode.attribute("x").as_int(),
					enemyNode.attribute("y").as_int()
				);

				// case 1 update position if: death=0 & savedDeath=0 
				if (xmlDeath == 0 && xmlSavedDeath == 0) {
					for (int i = 0; i < bossList.size(); ++i) {
						if (bossList[i]->GetRef() == xmlRef) {
							bossList[i]->SetPosition(pos);
							//bossList[i]->SetAliveInXML();
							//bossList[i]->SetEnabled(true);;
						}
					}
				}
				// case 2 create enemy if: death=0 & savedDeath=1 
				else if (xmlDeath == 1 && xmlSavedDeath == 0) {
					for (int i = 0; i < bossList.size(); ++i) {
						if (bossList[i]->GetRef() == xmlRef) {
							bossList[i]->SetPosition(pos);
							bossList[i]->SetAliveInXML();
							bossList[i]->SetEnabled(true);
							bossList[i]->ResetLives();
						}
					}
				}

				// case 3 do nothing if: death=1 & savedDeath=1
			}
		}
	}

}
//Dialogs
void Scene::DialogoOn() {Engine::GetInstance().entityManager->DialogoOn();}

void Scene::DialogoOff(){Engine::GetInstance().entityManager->DialogoOff();}
//block sensor
void Scene::DesbloquearSensor(){
	player->DesbloquearSensor();
	Engine::GetInstance().entityManager->AscensorOn();
}
void Scene::BloquearSensor() {
	player->BloquearSensor();
	Engine::GetInstance().entityManager->AscensorOff();
}

//hit player
void Scene::hitearPlayer() {player->hit();}


void Scene::SaveState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode;

	int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();
	if (currentScene == 1) {
		sceneNode = loadFile.child("config").child("scene");
	}
	if (currentScene == 2) {
		sceneNode = loadFile.child("config").child("scene2");
	}
	if (currentScene == 3) {
		sceneNode = loadFile.child("config").child("scene3");
	}
	if (currentScene == 4) {
		sceneNode = loadFile.child("config").child("scene4");
	}
	if (currentScene == 5) {
		sceneNode = loadFile.child("config").child("scene5");
	}
	if (currentScene == 6) {
		sceneNode = loadFile.child("config").child("scene6");
	}
	if (currentScene == 7) {
		sceneNode = loadFile.child("config").child("scene7");
	}
	if (currentScene == 8) {
		sceneNode = loadFile.child("config").child("scene8");
	}
	if (currentScene == 9) {
		sceneNode = loadFile.child("config").child("scene9");
	}
	if (currentScene == 10) {
		sceneNode = loadFile.child("config").child("scene10");
	}
	if (currentScene == 11) {
		sceneNode = loadFile.child("config").child("scene11");
	}


	//Save info to XML 

	//Player position
	sceneNode.child("entities").child("player").attribute("x").set_value(player->GetPosition().getX());
	sceneNode.child("entities").child("player").attribute("y").set_value(player->GetPosition().getY());

	//items
	pugi::xml_node itemsNode = sceneNode.child("entities").child("items");
	if (!itemList.empty()) {
		for (pugi::xml_node itemNode : itemsNode.children("item")) {
			std::string xmlRef = itemNode.attribute("ref").as_string();
			for (const auto& item : itemList) {
				if (item->GetRef() == xmlRef) {
					if (item->DeathValue != 0) {
						itemNode.attribute("savedDeath").set_value(1);
					}
					break;
				}
			}
		}
	}

	//enemies
	pugi::xml_node bossesNode = sceneNode.child("entities").child("enemies");
	if (!bossList.empty()) {
		int i = 0;
		for (pugi::xml_node bossNode : bossesNode.children("enemy")) {
			std::string type = bossNode.attribute("type").as_string();
			if (type == "boss") {
				if (i < bossList.size()) {
					std::string xmlRef = bossNode.attribute("ref").as_string();
					if (bossList[i]->DeathValue == 0) {
						for (int i = 0; i < bossList.size(); ++i) {
							if (bossList[i]->GetRef() == xmlRef) {
								bossNode.attribute("x").set_value(bossList[i]->GetPosition().getX());
								bossNode.attribute("y").set_value(bossList[i]->GetPosition().getY());
							}
						}

					}
					else bossNode.attribute("savedDeath").set_value(1);

					i++;
				}
			}
		}
	}

	//Saves the modifications to the XML 
	loadFile.save_file("config.xml");
}

bool Scene::OnGuiMouseClickEvent(GuiControl* control)//when you press the button
{


	switch (control->id)
	{
	case 2: // NewGame
		currentState = SceneState::TEXT_SCREEN;
		hasStartedGame = true;
		musicOn = false;
		NewGameReset();
		DisableMenuButtons();
		break;
	case 3: // Continue
		currentState = SceneState::GAMEPLAY;
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 1.0f);
		DisableMenuButtons();
		break;
	case 4: // Settings
		previousState = currentState;

		currentState = SceneState::SETTINGS_MENU;

		DisableMenuButtons();
		EnableFullscreenButton();
		EnableSettingsControls();
		break;
	case 5: // Credits
		currentState = SceneState::CREDITS_MENU;
		DisableMenuButtons();
		break;
	case 6: // Exit
		exitRequested = true;
		break;
	case 7: // ResumeGame 
		pauseMenuOn = false;
		SetCurrentState(SceneState::GAMEPLAY);
		break;
	case 8: // BackToTitle 
		SetCurrentState(SceneState::INTRO_SCREEN);
		introMusicPlaying = false;
		pauseMenuOn = false;
		DisablePauseButtons();
		break;
	case 9: // SettingsPause 
		previousState = currentState;
		currentState = SceneState::SETTINGS_MENU;
		DisablePauseButtons();
		EnableFullscreenButtonPauseMenu();
		EnableSettingsControlsPauseMenu();
		break;
	case 10: // ExitGamePause
		exitRequested = true;
		break;
	case 11: // FullScreen
		fullscreenState = !fullscreenState;

		Engine::GetInstance().window->ToggleFullscreen();
		//EnableFullscreenButton();
		//EnableFullscreenButtonPauseMenu();
		break;
	case 12: // ControlsPauseMenu
		previousState = currentState;
		currentState = SceneState::CONTROLS_MENU;
		DisablePauseButtons();
		break;
	case 13: // ControlsMainMenu
		previousState = currentState;
		currentState = SceneState::CONTROLS_MENU;
		DisableMenuButtons();
		break;
	case 14: {// Music slider
		int volume = control->GetValue();
		Engine::GetInstance().audio->SetMusicVolume(volume);

		// Sincronize the music slider in both menus
		if (musicSlider) musicSlider->SetValue(volume);
		if (musicSliderPauseMenu) musicSliderPauseMenu->SetValue(volume);
		break;
	}
	case 15: { // FX slider
		int volume = control->GetValue();
		Engine::GetInstance().audio->SetGlobalFxVolume(volume);

		// Sincronize the fx slider in both menus
		if (fxSlider) fxSlider->SetValue(volume);
		if (fxSliderPauseMenu) fxSliderPauseMenu->SetValue(volume);
		break;
	}
	}

	return true;
}

SceneState Scene::GetCurrentState() const
{
	return currentState;
}

void Scene::SetCurrentState(SceneState state)
{
	if (currentState == state) return;

	previousState = currentState;  

	if (previousState == SceneState::SETTINGS_MENU && state == SceneState::INTRO_SCREEN) {
		DisableFullscreenButton();
		DisableSettingsControls();
	}
	else if (previousState == SceneState::SETTINGS_MENU && state == SceneState::PAUSE_MENU) {
		DisableFullscreenButtonPauseMenu();
		DisableSettingsControlsPauseMenu();
	}
	else if (previousState == SceneState::PAUSE_MENU) {
		DisablePauseButtons();
	}
	else if (previousState == SceneState::INTRO_SCREEN) {
		DisableMenuButtons();
	}

	currentState = state;  

	if (state == SceneState::SETTINGS_MENU) {
		int currentMusicVol = Engine::GetInstance().audio->GetMusicVolume();
		int currentFxVol = Engine::GetInstance().audio->GetFxVolume();

		if (musicSlider) {
			musicSlider->SetValue(currentMusicVol);
			musicSlider->UpdateHandlePosition();
		}
		if (fxSlider) {
			fxSlider->SetValue(currentFxVol);
			fxSlider->UpdateHandlePosition();
		}

		if (musicSliderPauseMenu) {
			musicSliderPauseMenu->SetValue(currentMusicVol);
			musicSliderPauseMenu->UpdateHandlePosition();
		}
		if (fxSliderPauseMenu) {
			fxSliderPauseMenu->SetValue(currentFxVol);
			fxSliderPauseMenu->UpdateHandlePosition();
		}
	}

	if (state == SceneState::SETTINGS_MENU && previousState == SceneState::INTRO_SCREEN) {
		EnableFullscreenButton();
		EnableSettingsControls();
	}
	else if (state == SceneState::SETTINGS_MENU && previousState == SceneState::PAUSE_MENU) {
		EnableFullscreenButtonPauseMenu();
		EnableSettingsControlsPauseMenu();
	}
	else if (state == SceneState::INTRO_SCREEN) {
		EnableMenuButtons();
	}
	else if (state == SceneState::PAUSE_MENU) {
		CreatePauseMenu();
	}
}

void Scene::EnableSettingsControls()
{
	EnableFullscreenButton();
	if (musicSlider) musicSlider->SetState(GuiControlState::NORMAL);
	if (fxSlider) fxSlider->SetState(GuiControlState::NORMAL);
}

void Scene::DisableSettingsControls()
{
	DisableFullscreenButton();
	if (musicSlider) musicSlider->SetState(GuiControlState::DISABLED);
	if (fxSlider) fxSlider->SetState(GuiControlState::DISABLED);
}

void Scene::CreateFullscreenButton()
{
	if (!fullscreenButtonsCreated) {

		fullscreenButtonsCreated = true;
	}
}

void Scene::EnableFullscreenButton()
{
	if (FullScreenCheckbox) {
		FullScreenCheckbox->SetChecked(fullscreenState);
		//FullScreenCheckbox->SetState(GuiControlState::NORMAL);
	}
}

void Scene::DisableFullscreenButton()
{
	if (FullScreenCheckbox) {
		FullScreenCheckbox->SetState(GuiControlState::DISABLED);
	}
}

void Scene::EnableSettingsControlsPauseMenu()
{
	EnableFullscreenButtonPauseMenu();
	if (musicSliderPauseMenu) musicSliderPauseMenu->SetState(GuiControlState::NORMAL);
	if (fxSliderPauseMenu) fxSliderPauseMenu->SetState(GuiControlState::NORMAL);
}

void Scene::DisableSettingsControlsPauseMenu()
{
	DisableFullscreenButtonPauseMenu();
	if (musicSliderPauseMenu) musicSliderPauseMenu->SetState(GuiControlState::DISABLED);
	if (fxSliderPauseMenu) fxSliderPauseMenu->SetState(GuiControlState::DISABLED);
}

void Scene::EnableFullscreenButtonPauseMenu()
{
	if (FullScreenCheckboxPauseMenu) {
		FullScreenCheckboxPauseMenu->SetChecked(fullscreenState);
		//FullScreenCheckbox->SetState(GuiControlState::NORMAL);
	}
}

void Scene::DisableFullscreenButtonPauseMenu()
{
	if (FullScreenCheckboxPauseMenu) {
		FullScreenCheckboxPauseMenu->SetState(GuiControlState::DISABLED);
	}
}

void Scene::DrawCurrentScene()
{
	switch (currentState)
	{
	case SceneState::INTRO_SCREEN:
		if (introScreenTexture != nullptr)
		{
			Engine::GetInstance().render->DrawTexture(introScreenTexture, -50, 0, nullptr, 0.0f);
		}
		break;
	case SceneState::SETTINGS_MENU:
		if (settingsTexture != nullptr && previousState == SceneState::INTRO_SCREEN)
		{
			Engine::GetInstance().render->DrawTexture(settingsTexture, -50, 0, nullptr, 0.0f);
		}
		else if (settingsPauseMenu != nullptr && previousState == SceneState::PAUSE_MENU)
		{
			Engine::GetInstance().render->DrawTexture(settingsPauseMenu, -40, 0, nullptr, 0.0f);
		}
		break;
	case SceneState::CREDITS_MENU:
		if (creditsTexture != nullptr)
		{
			Engine::GetInstance().ffmpeg->ConvertPixels("Assets/Textures/GUI/CREDITS.mp4");
		}
		break;
	case SceneState::TEXT_SCREEN:
		break;
	case SceneState::GAMEPLAY:
		break;
	case SceneState::PAUSE_MENU:
		if (pauseTexture != nullptr) {
			Engine::GetInstance().render->DrawTexture(pauseTexture, -40, 0, nullptr, 0.0f);
		}
		break;
	case SceneState::CONTROLS_MENU:
		SDL_Texture* currentTexture = nullptr;
		switch (currentControlsPage) {
		case 0: 
			currentTexture = controlsTexture1; 
			break;
		case 1: 
			currentTexture = controlsTexture2; 
			break;
		case 2: 
			currentTexture = controlsTexture3; 
			break;
		}

		if (currentTexture != nullptr) {
			Engine::GetInstance().render->DrawTexture(currentTexture, -40, 0, nullptr, 0.0f);
		}
		break;
	}

}

void Scene::CreatePauseMenu() {
	if (!pauseButtonsCreated) {
		// Create the pause menu buttons
		ResumeGame = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 7, " ", { 580, 280, 120,20 }, this);
		ResumeGame->SetTextures(ResumeNormal, ResumeFocused, ResumePressed, ResumeOff);
		ResumeGame->SetState(GuiControlState::DISABLED);

		BackToTitle = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 8, " ", { 580, 330, 120,20 }, this);
		BackToTitle->SetTextures(BackToTitleNormal, BackToTitleFocused, BackToTitlePressed, BackToTitleOff);
		BackToTitle->SetState(GuiControlState::DISABLED);

		ControlsPauseMenu = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 12, " ", { 580, 380, 120,20 }, this);
		ControlsPauseMenu->SetTextures(ControlsNormal, ControlsFocused, ControlsPressed, ControlsOff);
		ControlsPauseMenu->SetState(GuiControlState::DISABLED);

		SettingsPause = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 9, " ", { 580, 430, 120,20 }, this);
		SettingsPause->SetTextures(SettingsNormal, SettingsFocused, SettingsPressed, SettingsOff);
		SettingsPause->SetState(GuiControlState::DISABLED);

		ExitGamePause = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 10, " ", { 580, 480, 120,20 }, this);
		ExitGamePause->SetTextures(ExitNormal, ExitFocused, ExitPressed, ExitOff);
		ExitGamePause->SetState(GuiControlState::DISABLED);

		// Fullscreen Checkbox
		fullscreenState = Engine::GetInstance().window->IsFullscreen();

		SDL_Rect fullscreenPos = { 520, 400, 40, 40 };
		FullScreenCheckboxPauseMenu = (CheckBox*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::CHECKBOX, 11, " ", fullscreenPos, this);
		FullScreenCheckboxPauseMenu->SetTextures(FullScreenNormal, FullScreenPressed);
		FullScreenCheckboxPauseMenu->SetState(GuiControlState::DISABLED);

		// Music Slider
		SDL_Rect musicSliderPos = { 580, 270, 200, 10 };
		musicSliderPauseMenu = (GuiControlSlider*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 14, " ", musicSliderPos, this, { 0, 128 });// min=0, max=128

		musicSliderPauseMenu->SetTextures(sliderBase, sliderHandle);
		musicSliderPauseMenu->SetHandleSize(20, 20); // Handle size
		int currentMusicVol = Engine::GetInstance().audio->GetMusicVolume();
		musicSliderPauseMenu->SetValue(currentMusicVol);
		musicSliderPauseMenu->UpdateHandlePosition();
		musicSliderPauseMenu->SetState(GuiControlState::DISABLED);

		// SFX Slider
		SDL_Rect sfxSliderPos = { 580, 310, 200, 10 };
		fxSliderPauseMenu = (GuiControlSlider*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, 15, " ", sfxSliderPos, this, { 0, 128 });// min=0, max=128
		fxSliderPauseMenu->SetTextures(sliderBase, sliderHandle);
		fxSliderPauseMenu->SetHandleSize(20, 20);
		fxSliderPauseMenu->SetValue(Engine::GetInstance().audio->GetFxVolume());
		fxSliderPauseMenu->UpdateHandlePosition();
		fxSliderPauseMenu->SetState(GuiControlState::DISABLED);

		pauseButtonsCreated = true;

	}
	EnablePauseButtons();

}
void Scene::DisableMenuButtons() {
	if (NewGame) NewGame->SetState(GuiControlState::DISABLED);
	if (Continue) Continue->SetState(GuiControlState::DISABLED);
	if (ControlsMainMenu) ControlsMainMenu->SetState(GuiControlState::DISABLED);
	if (Settings) Settings->SetState(GuiControlState::DISABLED);
	if (Credits) Credits->SetState(GuiControlState::DISABLED);
	if (ExitGame) ExitGame->SetState(GuiControlState::DISABLED);
}

void Scene::EnableMenuButtons() {
	if (NewGame) NewGame->SetState(GuiControlState::NORMAL);
	if (Continue) {
		if (hasStartedGame) {
			Continue->SetState(GuiControlState::NORMAL);
		}
		else {
			Continue->SetState(GuiControlState::OFF);
		}
	}
	if (ControlsMainMenu) ControlsMainMenu->SetState(GuiControlState::NORMAL);
	if (Settings) Settings->SetState(GuiControlState::NORMAL);
	if (Credits) Credits->SetState(GuiControlState::NORMAL);
	if (ExitGame) ExitGame->SetState(GuiControlState::NORMAL);
}

void Scene::DisablePauseButtons() {
	if (ResumeGame) ResumeGame->SetState(GuiControlState::DISABLED);
	if (BackToTitle) BackToTitle->SetState(GuiControlState::DISABLED);
	if (ControlsPauseMenu) ControlsPauseMenu->SetState(GuiControlState::DISABLED);
	if (SettingsPause) SettingsPause->SetState(GuiControlState::DISABLED);
	if (ExitGamePause) ExitGamePause->SetState(GuiControlState::DISABLED);
}

void Scene::EnablePauseButtons() {
	if (ResumeGame) ResumeGame->SetState(GuiControlState::NORMAL);
	if (BackToTitle) BackToTitle->SetState(GuiControlState::NORMAL);
	if (ControlsPauseMenu) ControlsPauseMenu->SetState(GuiControlState::NORMAL);
	if (SettingsPause) SettingsPause->SetState(GuiControlState::NORMAL);
	if (ExitGamePause) ExitGamePause->SetState(GuiControlState::NORMAL);
}

void Scene::NewGameReset() {
	player->ResetToInitPosition();

	// Reset the player position to the initial position
	Engine::GetInstance().sceneLoader->LoadScene(1, 2877, 2048, false, false);

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	// Reset lastCheckpointScene to 1 if it's not already 1
	pugi::xml_node gamestateNode = loadFile.child("config").child("gamestate");
	pugi::xml_node lastCheckpointNode = gamestateNode.child("lastCheckpointScene").child("scene");

	int currentValue = lastCheckpointNode.attribute("value").as_int();
	if (currentValue != 1) {
		lastCheckpointNode.attribute("value").set_value(1);
	}
	pugi::xml_node sceneNode;
	int maxScenes = 12;
	for (int i = 0; i < maxScenes; ++i) {
		if (i == 0) { sceneNode = loadFile.child("config").child("scene"); }
		else if (i == 1) {
			sceneNode = loadFile.child("config").child("scene2");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 2) {
			sceneNode = loadFile.child("config").child("scene3");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 3) {
			sceneNode = loadFile.child("config").child("scene4");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 4) {
			sceneNode = loadFile.child("config").child("scene5");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 5) {
			sceneNode = loadFile.child("config").child("scene6");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 6) {
			sceneNode = loadFile.child("config").child("scene7");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 7) {
			sceneNode = loadFile.child("config").child("scene8");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 8) {
			sceneNode = loadFile.child("config").child("scene9");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 9) {
			sceneNode = loadFile.child("config").child("scene10");/*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 10) {
			sceneNode = loadFile.child("config").child("scene11"); /*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}
		else if (i == 11) {
			sceneNode = loadFile.child("config").child("scene12"); /*visibility map*/sceneNode.child("visibility").attribute("accessed").set_value(false);
		}

		//checkpoints
		pugi::xml_node checkpointsNode = sceneNode.child("entities").child("checkpoints");

		for (pugi::xml_node checkpointNode : checkpointsNode.children("checkpoint")) {
			checkpointNode.attribute("pendingToChangeAnim").set_value(false);
			checkpointNode.attribute("activated").set_value(false);
		}

		//item
		pugi::xml_node itemsNode = sceneNode.child("entities").child("items");

		for (pugi::xml_node itemNode : itemsNode.children("item")) {
			std::string name = itemNode.attribute("name").as_string();
			if (name == "Whip") {
				itemNode.attribute("created").set_value(false);
			}
			itemNode.attribute("death").set_value(0);
			itemNode.attribute("savedDeath").set_value(0);
		}

		//enemies
		pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");

		for (pugi::xml_node enemyNode : enemiesNode.children("enemy")) {
			std::string type = enemyNode.attribute("type").as_string();
			if (type == "boss") {
				if (i == 2) {
					enemyNode.attribute("x").set_value(2045);
					enemyNode.attribute("y").set_value(1648);
				}

				enemyNode.attribute("death").set_value(0);
				enemyNode.attribute("savedDeath").set_value(0);
			}
		}
	}

	pugi::xml_node windowNode = loadFile.child("config").child("window");
	if (windowNode) {
		pugi::xml_node fullscreenNode = windowNode.child("fullscreen_window");
		if (fullscreenNode) {
			fullscreenNode.attribute("value").set_value(false);
		}
	}

	loadFile.save_file("config.xml");
}