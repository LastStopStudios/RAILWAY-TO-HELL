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

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN){
		Mostrar = !Mostrar;
		LoadMap();
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

	SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);
	SDL_RenderCopy(Engine::GetInstance().render->renderer, mapa, nullptr, &dstRect);
	SDL_RenderCopy(Engine::GetInstance().render->renderer, pj, nullptr, &dstRect2);
}

void Mapa::LoadMap(){
	Engine::GetInstance().scene->DialogoOn(); // stop entities

	//Scene on screen
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel(); //bring out the current scene
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);//pass the scene from where to get the dialogues

	//Map properties
	posx = 200;//player position X
	posy = 200;//player positon Y
	Engine::GetInstance().window.get()->GetWindowSize(w, h);//Screen size

	for (const auto& mapardo : mapardo) { // Iterate through all scenes
		if (mapardo.escena == scene) { // Check where the player needs to go
			mapaC = mapardo.mapardo; // map to load
		}
	}
	
	
	

	//Textures to load
	fondo = Engine::GetInstance().textures->Load("Assets/Textures/mapa/MapBackground.png "); //Load texture for map background
	mapa = Engine::GetInstance().textures->Load(mapaC.c_str()); //Load texture for map
	pj = Engine::GetInstance().textures->Load("Assets/Textures/mapa/pj2.png "); //Load texture for character
}