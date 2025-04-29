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
	}

	// Check controller input for jump (X button)
	if (Engine::GetInstance().IsControllerConnected()) {
		SDL_GameController* controller = Engine::GetInstance().GetGameController();
		bool AbuttonPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
		// Store previous button state to detect when it is newly pressed
		static bool previousAbuttonPressed = false;
		if (AbuttonPressed && !previousAbuttonPressed) {
			Mostrar = !Mostrar;
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
	return true;
}

void Mapa::ShowMap() {
	int w, h;
	Engine::GetInstance().window.get()->GetWindowSize(w, h);//Screen size
	SDL_Rect dstRect = { 0, 0, w, h }; //Position and scale text background

	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel(); //bring out the current scene
	scene = (currentLvl == 1) ? "scene" : "scene" + std::to_string(currentLvl);//pass the scene from where to get the dialogues

	fondo = Engine::GetInstance().textures->Load("Assets/Textures/IntroTexto.png "); //Load texture for text background
	SDL_RenderCopy(Engine::GetInstance().render->renderer, fondo, nullptr, &dstRect);
}