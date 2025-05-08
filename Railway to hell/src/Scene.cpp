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

Scene::Scene() : Module()
{
	name = "scene";
	currentState = SceneState::INTRO_SCREEN;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

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
		else {
		Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
		item->SetParameters(itemNode);
		//item->SetAliveInXML();
		itemList.push_back(item);
		}

	}

	for (pugi::xml_node projectileNode = configParameters.child("entities").child("projectiles").child("projectile"); projectileNode; projectileNode = projectileNode.next_sibling("projectile"))
	{
		std::string type = projectileNode.attribute("name").as_string();
		if (type == "ball")	ballConfigNode = projectileNode;
		else {

		}
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

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		std::string type = enemyNode.attribute("type").as_string(); 
		if (type == "rastrero") {
			Terrestre* enemy = (Terrestre*)Engine::GetInstance().entityManager->CreateEntity(EntityType::TERRESTRE);
			enemy->SetParameters(enemyNode);
			enemyList.push_back(enemy);
		}
		if (type == "volador") {
			Volador* volador = (Volador*)Engine::GetInstance().entityManager->CreateEntity(EntityType::VOLADOR);
			volador->SetParameters(enemyNode);
			voladorList.push_back(volador);  
		}
		if (type == "boss") {
			Boss* boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
			boss->SetParameters(enemyNode);
			bossList.push_back(boss);
		}
		if (type == "guardian") {
			Caronte* caronte = (Caronte*)Engine::GetInstance().entityManager->CreateEntity(EntityType::CARONTE);
			caronte->SetParameters(enemyNode);
			caronteList.push_back(caronte);
		}
	}

	// Instantiate a new GuiControlButton in the Scene
	// Button
//	SDL_Rect btPos = { 520, 350, 120,20 };
//	guiBt = (GuiControlButton*) Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "MyButton", btPos, this);

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//Cargar Texturas splash screen
	introScreenTexture = Engine::GetInstance().textures->Load("Assets/Textures/SplashScreen.png");
	introTextoTexture = Engine::GetInstance().textures->Load("Assets/Textures/IntroTexto.png");
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

		if (!introMusicPlaying) {

			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Intro.ogg", 1.0f);
			Engine::GetInstance().audio.get()->SetMusicVolume(2);
			Engine::GetInstance().sceneLoader->FadeOut(2.5f, false);// Animation speed (FadeOut)

			introMusicPlaying = true;
			textMusicPlaying = false;
			currentMusic = "intro";
		}
		if (introMusicPlaying) {
			DrawCurrentScene();
		}
		// Original keyboard input
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
			Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
			currentState = SceneState::TEXT_SCREEN;
			skipFirstInput = true;
		}
		//Check controller input
		if (Engine::GetInstance().IsControllerConnected()) {
			SDL_GameController* controller = Engine::GetInstance().GetGameController();
			// SDL_CONTROLLER_BUTTON_A corresponds to the bottom button (X on PlayStation, A on Xbox)
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
				Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
				currentState = SceneState::TEXT_SCREEN;
				skipFirstInput = true;
			}
		}
		break;
	case SceneState::TEXT_SCREEN:

		if (!textMusicPlaying) {
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Nothing.ogg", 1.0f);
			textMusicPlaying = true;
			introMusicPlaying = false;
			currentMusic = "text";
			Engine::GetInstance().sceneLoader->FadeOut(2.5, false);// Animation speed (FadeOut)
		}
		if (textMusicPlaying) {
			DrawCurrentScene();
		}
		// Original keyboard input
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
			Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
			currentState = SceneState::GAMEPLAY;
		}
		//Check controller input
		if (Engine::GetInstance().IsControllerConnected()) {
			SDL_GameController* controller = Engine::GetInstance().GetGameController();
			// SDL_CONTROLLER_BUTTON_A corresponds to the bottom button (X on PlayStation, A on Xbox)
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
				Engine::GetInstance().sceneLoader->FadeIn(2.5f);// Animation speed (FadeIn)
				currentState = SceneState::GAMEPLAY;
			}
		}
		break;
	case SceneState::GAMEPLAY:
		if (currentMusic == "text") {
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/caronte.ogg", 1.0f);
			currentMusic = "";
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
		dibujar = true;
		//If mouse button is pressed modify enemy position
		/*if (Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_DOWN) {
		enemyList[0]->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
		enemyList[0]->ResetPath();
		}*/

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
		if(BossBattle == false){
			//Camera follows player
			Engine::GetInstance().render.get()->camera.x = player->position.getX() * -1.0f + 340.0f;
			Engine::GetInstance().render.get()->camera.y = player->position.getY() * -1.0f + 485.0f;
		}else if(BossBattle == true){
			//Camera for Boss Battle
			for (const auto& Bosses : Bosses) { // Iterate through all scenes
				if (Bosses.id == Engine::GetInstance().sceneLoader->GetCurrentLevel()) {//scene id equals scene we are
					Engine::GetInstance().render.get()->camera.x = - Bosses.x;// position X of the camera
					Engine::GetInstance().render.get()->camera.y = - Bosses.y;// position Y of the camera
				}
			}
		}
		
	}

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		LoadState();

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		SaveState();

	return ret;
}

void Scene::EntrarBoss() { BossBattle = true; LOG("Entra al Boss"); }

void Scene::SalirBoss() { BossBattle = false; LOG("Sale del Boss");
}

// Called before quitting
bool Scene::CleanUp()
{
	if (introScreenTexture != nullptr)
	{
		Engine::GetInstance().textures->UnLoad(introScreenTexture);
		introScreenTexture = nullptr;
	}
	if (introTextoTexture != nullptr)
	{
		Engine::GetInstance().textures->UnLoad(introTextoTexture);
		introTextoTexture = nullptr;
	}

	player->ResetToInitPosition();

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return false;
	}

	pugi::xml_node sceneNode;
	int maxScenes = 3;
	for (int i = 0; i < 3; ++i) {
		if (i == 0) sceneNode = loadFile.child("config").child("scene");
		else if (i == 1) sceneNode = loadFile.child("config").child("scene2");
		else if (i == 2) sceneNode = loadFile.child("config").child("scene3");

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

void Scene::DialogoOn() {
	Engine::GetInstance().entityManager->DialogoOn();
}

void Scene::DialogoOff(){
	Engine::GetInstance().entityManager->DialogoOff();
}

void Scene::DesbloquearSensor(){
	player->DesbloquearSensor();
	Engine::GetInstance().entityManager->AscensorOn();
}

void Scene::BloquearSensor() {
	player->BloquearSensor();
	Engine::GetInstance().entityManager->AscensorOff();
}

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


	if (control->id == 1) {
		Engine::GetInstance().dialogoM->Texto("1"); // Call Text
	}
	return true;
}

SceneState Scene::GetCurrentState() const
{
	return currentState;
}

void Scene::SetCurrentState(SceneState state)
{
	currentState = state;
}

void Scene::DrawCurrentScene()
{
	switch (currentState)
	{
	case SceneState::INTRO_SCREEN:
		if (introScreenTexture != nullptr)
		{
			Engine::GetInstance().render->DrawTexture(introScreenTexture, 0, 0);
		}
		break;
	case SceneState::TEXT_SCREEN:
		if (introTextoTexture != nullptr)
		{
			Engine::GetInstance().render->DrawTexture(introTextoTexture, 0, 0);
		}
		break;
	}
}