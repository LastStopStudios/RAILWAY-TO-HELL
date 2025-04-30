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
			if (!Mostrar) {
				Engine::GetInstance().scene->DialogoOff(); // Return control to all entities
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
	if (fondo != nullptr) {
		Engine::GetInstance().textures->UnLoad(fondo);//Unload text background
		fondo = nullptr;
	}
	if (mapa != nullptr) {
		Engine::GetInstance().textures->UnLoad(mapa);//Unload text background
		mapa = nullptr;
	}
	if (pj != nullptr) {
		Engine::GetInstance().textures->UnLoad(pj);//Unload text background
		mapa = nullptr;
	}
	if (cobertura != nullptr) {
		Engine::GetInstance().textures->UnLoad(cobertura);//Unload text background
		mapa = nullptr;
	}
	return true;
}

void Mapa::ShowMap() {
	SDL_Rect dstRect = { 0, 0, w, h }; //Position and scale of background and map texture
	SDL_Rect dstRect2 = { posx,posy, 40, 40 }; //Position and scale character icon
	SDL_Rect dstRect3 = { 500,200, 200, 200 }; //Position and scale of the black texture to covern unknown zones of the map

	SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);//render map background
	SDL_RenderCopy(Engine::GetInstance().render->renderer, mapa, nullptr, &dstRect);//render map
	SDL_RenderCopy(Engine::GetInstance().render->renderer, pj, nullptr, &dstRect2);//render character icon
	//SDL_RenderCopy(Engine::GetInstance().render->renderer, cobertura, nullptr, &dstRect3);//render black rectangle do more to cover the map 
}

void Mapa::LoadMap(){
	Engine::GetInstance().scene->DialogoOn(); // stop entities

	//Scene on screen
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel(); //bring out the current scene
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);//pass the scene from where to get the dialogues

	//player posiion
	Vector2D playerpos = Engine::GetInstance().scene->GetPlayerPosition();
	LOG("X player: %f", playerpos.getX());
	LOG("Y player: %f", playerpos.getY());

	//Screen size
	Engine::GetInstance().window.get()->GetWindowSize(w, h);

	for (const auto& zonaspj : zonaspj) { // Iterate through all the maps
		if (zonaspj.escena == scene) { // Check what scene is currently on screen
			if(zonaspj.x1 < playerpos.getX() && playerpos.getX() < zonaspj.x2 && zonaspj.y1 < playerpos.getY() && playerpos.getY() < zonaspj.y2) {
				//Icon position on map
				posx = zonaspj.playerx;
				posy = zonaspj.playery;
				break;
			}
		}
	}

	//Textures to load
	fondo = Engine::GetInstance().textures->Load("Assets/Textures/mapa/MapBackground.png "); //Load texture for map background
	mapa = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Mapa.png"); //Load texture for map
	cobertura = Engine::GetInstance().textures->Load("Assets/Textures/mapa/Cobertura.png"); //Load black texture to cover unknow parts of the map
	pj = Engine::GetInstance().textures->Load("Assets/Textures/mapa/pj2.png"); //Load texture for character
}

void Mapa::DialogoOn() { dialogoOn = true; }//stop map from showing during dialogs
void Mapa::DialogoOff() { dialogoOn = false; }//habilitates the map