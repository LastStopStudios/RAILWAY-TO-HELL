#include "UI.h"
#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"


UI::UI() : Module()
{
	name = "ui";
}

//Destructor
UI::~UI()
{
	CleanUp();
}

//Called before render is available
bool UI::Awake()
{
	return true;
}

//Called before the first frame
bool UI::Start()
{

	return true;
}

//Called each loop iteration
bool UI::PreUpdate()
{

	return true;
}

// Called each loop iteration
bool UI::Update(float dt) {

	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
		Engine::GetInstance().scene->ResetSkipInput();
		return true;
	}
	

	return true;
}

bool UI::PostUpdate()
{
	
	return true;
}

bool UI::CleanUp()
{
	if (vida != nullptr) {
		Engine::GetInstance().textures->UnLoad(vida);//Unload text background
		vida = nullptr;
	}
	if (amo != nullptr) {
		Engine::GetInstance().textures->UnLoad(amo);//Unload text background
		amo = nullptr;
	}
	return true;
}

void UI::LoadTUi()
{
	//vida = Engine::GetInstance().textures->Load("Assets/Textures/ "); //Load life of the player
	//amo = Engine::GetInstance().textures->Load("Assets/Textures/ "); //Load stamina of the player

	if(figth){/*boss = Engine::GetInstance().textures->Load("Assets/Textures/ "); //Load life of the player*/}
}

void UI::renderUI()
{
	SDL_Rect dstRect = { posx,posy, w, h }; //Position and scale character health
	SDL_Rect dstRect2 = { posx2,posy2, w2, h2 }; //Position and scale character Amo

	SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
	SDL_RenderCopy(Engine::GetInstance().render->renderer, amo, nullptr, &dstRect2);//render  character health
}

void UI::Boss()
{
	SDL_Rect dstRect2 = { posxb,posyb, wb, hb }; //Position and scale Boss health
	
}


