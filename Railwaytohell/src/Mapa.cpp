#include "Mapa.h"
#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"



Mapa::Mapa() : Module()
{
	name = "mapa";
}

//Destructor
Mapa::~Mapa()
{
	CleanUp();
}

//Called before render is available
bool Mapa::Awake()
{
	return true;
}

//Called before the first frame
bool Mapa::Start()
{
	Mostrar = false;
	remember1 = false;
	remember2 = false;
	remember3 = false;
	remember4 = false;
	remember5 = false;
	remember6 = false;
	remember7 = false;
	remember8 = false;
	return true;
}

//Called each loop iteration
bool Mapa::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool Mapa::Update(float dt) {

	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
		Engine::GetInstance().scene->ResetSkipInput();
		return true;
	}
	if (dialogoOn){ return true; }
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN) {
		Mostrar = !Mostrar;
		if (Mostrar) { LoadMap(); }
		if(!Mostrar){
			Engine::GetInstance().scene->DialogoOff(); // Return control to all entities
			CleanUp();
		}
	}

	// Check controller input for jump (X button)
	if (Engine::GetInstance().IsControllerConnected()) {
		SDL_GameController* controller = Engine::GetInstance().GetGameController();
		bool AbuttonPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
		// Store previous button state to detect when it is newly pressed
		static bool previousAbuttonPressed = false;
		if (AbuttonPressed && !previousAbuttonPressed) {
			Mostrar = !Mostrar;
			if (Mostrar) { LoadMap(); }
			if (!Mostrar) {
				Engine::GetInstance().scene->DialogoOff(); // Return control to all entities
				CleanUp();
			}
		}
		previousAbuttonPressed = AbuttonPressed;
	}
	
	return true;
}

bool Mapa::PostUpdate()
{
	if (Mostrar) {
		ShowMap();
		
	}
	return true;
}

bool Mapa::CleanUp()
{
	if (mapa != nullptr) {
		Engine::GetInstance().textures->UnLoad(mapa);//Unload text background
		mapa = nullptr;
	}
/*	if (pj != nullptr) {
		Engine::GetInstance().textures->UnLoad(pj);//Unload text background
		pj = nullptr;
	}*/
	if (MNegro != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro);//Unload text background
		MNegro = nullptr;
	}
	if (MNegro2 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro2);//Unload text background
		MNegro2 = nullptr;
	}
	if (MNegro3 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro3);//Unload text background
		MNegro3 = nullptr;
	}

	if (MNegro4 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro4);//Unload text background
		MNegro4 = nullptr;
	}
	if (MNegro5 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro5);//Unload text background
		MNegro5 = nullptr;
	}
	if (MNegro6 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro6);//Unload text background
		MNegro6 = nullptr;
	}
	if (MNegro7 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro7);//Unload text background
		MNegro7 = nullptr;
	}
	if (MNegro8 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro8);//Unload text background
		MNegro8 = nullptr;
	}
	if (MNegro9 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro9);//Unload text background
		MNegro9 = nullptr;
	}
	if (MNegro10 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro10);//Unload text background
		MNegro10 = nullptr;
	}
	if (MNegro12 != nullptr) {
		Engine::GetInstance().textures->UnLoad(MNegro12);//Unload text background
		MNegro12 = nullptr;
	}
	if (Recuerdo1 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo1);//Unload text background
		Recuerdo1 = nullptr;
	}
	if (Recuerdo2 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo2);//Unload text background
		Recuerdo2 = nullptr;
	}
	if (Recuerdo3 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo3);//Unload text background
		Recuerdo3 = nullptr;
	}
	if (Recuerdo4 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo4);//Unload text background
		Recuerdo4 = nullptr;
	}
	if (Recuerdo5 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo5);//Unload text background
		Recuerdo5 = nullptr;
	}
	if (Recuerdo6!= nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo6);//Unload text background
		Recuerdo6 = nullptr;
	}
	if (Recuerdo7 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo7);//Unload text background
		Recuerdo7 = nullptr;
	}
	if (Recuerdo8 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Recuerdo8);//Unload text background
		Recuerdo8 = nullptr;
	}

	return true;
}

void Mapa::ShowMap() {
	SDL_Rect dstRect = { 0, 0, w, h }; //Position and scale of background and map texture
	//SDL_Rect dstRect2 = { posx,posy, 40, 40 }; //Position and scale character icon
	

	SDL_RenderCopy(Engine::GetInstance().render->renderer, mapa, nullptr, &dstRect);//render map
	//SDL_RenderCopy(Engine::GetInstance().render->renderer, pj, nullptr, &dstRect2);//render character icon
	for (const auto& negro : negro)// Iterate through all the maps
	{
		if (!negro.visible) {
			switch (negro.num) {
			case 1: // Scene 
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro9, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 2: // Scene 2
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro2, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 3: // Scene 3
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro4, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 4: // Scene 4
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 5: // Scene 5
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro3, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 6: // Scene 6
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro6, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 7: // Scene 
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro10, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 8: // Scene 
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro7, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 9: // Scene 
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro12, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 10: // Scene 
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro8, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			case 11: // Scene 
				SDL_RenderCopy(Engine::GetInstance().render->renderer, MNegro5, nullptr, &dstRect);//render black rectangle do more to cover the map 
				break;
			}
			
		}
	}
	Remember();
	
}

void Mapa::CoverXML() {
	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	pugi::xml_node sceneNode = loadFile.child("config");
	i = 1;
	for (i = 1; i < 11; i++)// Iterate through all the maps
	{
		std::string escena = (i == 1) ? "scene" : "scene" + std::to_string(i);//pass the scene from where to get the dialogues
		pugi::xml_node sceneNodes = sceneNode.child(escena.c_str()).child("visibility");

		LOG("XML %d", sceneNodes.attribute("accessed").as_bool());
		bool visible = sceneNodes.attribute("accessed").as_bool();	
		for (size_t i = 0; i < negro.size(); ++i) {
			if (negro[i].escena == escena) { // Check what scene is currently on screen
				negro[i].visible = visible;

			}
		}
	}
	
}

void Mapa::LoadMap(){
	Engine::GetInstance().scene->DialogoOn(); // stop entities

	//Scene on screen
/*nt currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel(); //bring out the current scene
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);*/

	//player posiion
/*	Vector2D playerpos = Engine::GetInstance().scene->GetPlayerPosition();
	LOG("X player: %f", playerpos.getX());
	LOG("Y player: %f", playerpos.getY());
*/
	//Screen size
	Engine::GetInstance().window.get()->GetWindowSize(w, h);

	/*for (const auto& zonaspj : zonaspj) { // Iterate through all the maps
		if (zonaspj.escena == scene) { // Check what scene is currently on screen
			if(zonaspj.x1 < playerpos.getX() && playerpos.getX() < zonaspj.x2 && zonaspj.y1 < playerpos.getY() && playerpos.getY() < zonaspj.y2) {
				//Icon position on map
				posx = zonaspj.playerx;
				posy = zonaspj.playery;
				break;
			}
		}
	}*/
	CoverXML(); //needs fixing

	//Textures to load
	mapa = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Base.png"); //Load texture for map
	MNegro = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L2_P4.png"); //Load black texture to cover unknow parts of the map
	MNegro2 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L1_P4.png"); //Load black texture to cover unknow parts of the map
	MNegro3 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L2_P2.png"); //Load black texture to cover unknow parts of the map
	MNegro4 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L1_P2.png"); //Load black texture to cover unknow parts of the map
	MNegro5 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L2_P3.png"); //Load black texture to cover unknow parts of the map
	MNegro6 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_Central.png"); //Load black texture to cover unknow parts of the map
	MNegro7 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L2_P1.png"); //Load black texture to cover unknow parts of the map
	MNegro8 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L1_P3.png"); //Load black texture to cover unknow parts of the map
	MNegro9 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_L1_P1.png"); //Load black texture to cover unknow parts of the map
	MNegro10 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_Connection_A.png"); //Load black texture to cover unknow parts of the map
	MNegro12 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Map_Connection_B.png"); //Load black texture to cover unknow parts of the map ;
	//pj = Engine::GetInstance().textures->Load("Assets/Textures/mapa/pj2.png"); //Load texture for character


	Recuerdo1 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_01.png"); //Load Remember 1 Shoes
	Recuerdo3 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_03.png"); //Load Remember 2 Ticket Show
	Recuerdo6 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_06.png"); //Load Remember 3 Park
	Recuerdo7 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_07.png"); //Load Remember 4 Train railway
	Recuerdo2 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_02.png"); //Load Remember 5 Puzzle
	Recuerdo4 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_04.png"); //Load Remember 6 Ice Cream
	Recuerdo5 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_05.png"); //Load Remember 7 Snowman
	Recuerdo8 = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Menu_Inventory_Remembrance_08.png"); //Load Remember 8 Music
}

void Mapa ::Remember() {
	SDL_Rect dstRect = { 0, 0, w, h }; //Position and scale of background and map texture	
	if (remember1) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo1, nullptr, &dstRect);//render remember 1 texture
	}
	if (remember2) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo2, nullptr, &dstRect);//render remember 2 texture
	}
	if (remember3) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo3, nullptr, &dstRect);//render remember 3 texture
	}
	if (remember4) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo4, nullptr, &dstRect);//render remember 4 texture
	}
	if (remember5) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo5, nullptr, &dstRect);//render remember 5 texture
	}
	if (remember6) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo6, nullptr, &dstRect);//render remember 6 texture
	}
	if (remember7) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo7, nullptr, &dstRect);//render remember 7 texture
	}
	if (remember8) {
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Recuerdo8, nullptr, &dstRect);//render remember 8 texture
	}
}

void Mapa::DialogoOn() { dialogoOn = true; }//stop map from showing during dialogs
void Mapa::DialogoOff() { dialogoOn = false; }//habilitates the map