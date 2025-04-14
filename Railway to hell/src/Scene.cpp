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
			itemConfigNode = itemNode;
		}
		else {
		Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
		item->SetParameters(itemNode);
		itemList.push_back(item);
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
	SDL_Rect btPos = { 520, 350, 120,20 };
	guiBt = (GuiControlButton*) Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "MyButton", btPos, this);

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//Cargar Texturas splash screen
	introScreenTexture = Engine::GetInstance().textures->Load("Assets/Textures/SplashScreen.png");
	introTextoTexture = Engine::GetInstance().textures->Load("Assets/Textures/IntroTexto.png");
	//L06 TODO 3: Call the function to load the map. 
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
		if (introScreenTexture != nullptr)
		{
			Engine::GetInstance().render->DrawTexture(introScreenTexture, 0, 0);
		}
		if (!introMusicPlaying) {
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Intro.ogg", 1.0f);
			Engine::GetInstance().audio.get()->SetMusicVolume(2);
			introMusicPlaying = true;
			textMusicPlaying = false;
			currentMusic = "intro";
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
			currentState = SceneState::TEXT_SCREEN;
			skipFirstInput = true;
		}
		break;
	case SceneState::TEXT_SCREEN:
		if (!textMusicPlaying) {
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Text.ogg", 1.0f);
			textMusicPlaying = true;
			introMusicPlaying = false;
			currentMusic = "text";
		}
		Engine::GetInstance().render->DrawTexture(introTextoTexture, 0, 0);

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {

		currentState = SceneState::GAMEPLAY;
		}
		break;
	case SceneState::GAMEPLAY:
		if (currentMusic == "text") {
			Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Nothing.ogg", 1.0f);
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
		Engine::GetInstance().render.get()->DrawTexture(mouseTileTex,
			highlightTile.getX(),
			highlightTile.getY(),
			&rect);

		// saves the tile pos for debugging purposes
		if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
			tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + "," + std::to_string((int)mouseTile.getY()) + "] ";
			once = true;
		}

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
			Engine::GetInstance().render.get()->camera.x = player->position.getX() * -1.0f + 340.0f;
			Engine::GetInstance().render.get()->camera.y = player->position.getY() * -1.0f + 576.0f;
		}else if(BossBattle == true){
			for (const auto& Bosses : Bosses) { // Iterate through all scenes
				if (Bosses.id == Engine::GetInstance().sceneLoader->GetCurrentLevel()) {
					Engine::GetInstance().render.get()->camera.x = - Bosses.x;
					Engine::GetInstance().render.get()->camera.y = - Bosses.y;
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

	if(result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode = loadFile.child("config").child("scene");

	//Read XML and restore information

	//Player position
	Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int(),
								  sceneNode.child("entities").child("player").attribute("y").as_int());
	player->SetPosition(playerPos);

	//enemies
	// ...

}

void Scene::DialogoOn() {
	Engine::GetInstance().entityManager->DialogoOn();
}

void Scene::DialogoOff(){
	Engine::GetInstance().entityManager->DialogoOff();
}

void Scene::DesbloquearSensor(){
	player->DesbloquearSensor();
}

void Scene::BloquearSensor() {
	player->BloquearSensor();
}

void Scene::SaveState() {

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}

	pugi::xml_node sceneNode = loadFile.child("config").child("scene");

	//Save info to XML 

	//Player position
	sceneNode.child("entities").child("player").attribute("x").set_value(player->GetPosition().getX());
	sceneNode.child("entities").child("player").attribute("y").set_value(player->GetPosition().getY());

	//enemies
	// ...

	//Saves the modifications to the XML 
	loadFile.save_file("config.xml");
}

bool Scene::OnGuiMouseClickEvent(GuiControl* control)//al darle al boton
{


	if (control->id == 1) {
		Engine::GetInstance().dialogoM->Texto("1"); // Llama a Texto que toque
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