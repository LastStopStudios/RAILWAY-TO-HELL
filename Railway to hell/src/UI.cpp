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
	LoadTUi();
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
	
	renderUI();
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
	if (vida2 != nullptr) {
		Engine::GetInstance().textures->UnLoad(vida2);//Unload text background
		vida2 = nullptr;
	}
	if (Evida != nullptr) {
		Engine::GetInstance().textures->UnLoad(Evida);//Unload text background
		Evida = nullptr;
	}
	if (Evida2 != nullptr) {
		Engine::GetInstance().textures->UnLoad(Evida2);//Unload text background
		Evida2 = nullptr;
	}
	if (amo != nullptr) {
		Engine::GetInstance().textures->UnLoad(amo);//Unload text background
		amo = nullptr;
	}
	return true;
}

void UI::LoadTUi()
{
	vida = Engine::GetInstance().textures->Load("Assets/Textures/UI/Health_Segment_A.png"); //Load life of the player
	vida2 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Health_Segment_B.png"); //Load life of the player
	Evida2 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Health_Background_B.png"); //Load life of the player
	Evida = Engine::GetInstance().textures->Load("Assets/Textures/UI/Health_Background_A.png"); //Load Empty life of the player
	//amo = Engine::GetInstance().textures->Load("Assets/Textures/UI "); //Load stamina of the player
	//boss = Engine::GetInstance().textures->Load("Assets/Textures/UI "); //Load life of the player

}

void UI::renderUI()
{
	PJs();
	//SDL_RenderCopy(Engine::GetInstance().render->renderer, amo, nullptr, &dstRect2);//render  character health
	if (figth) { Boss1(); }
}

void UI::PJs() {
	SDL_Rect dstRect = { posx,posy, w, h }; //Position and scale character health
	SDL_Rect dstRect2 = { posx2,posy, w, h }; //Position and scale character health
	SDL_Rect dstRect3 = { posx3,posy, w, h }; //Position and scale character health
	SDL_Rect dstRect4 = { posx4,posy, w, h }; //Position and scale character health
	SDL_Rect dstRect5 = { posx5,posy, w, h }; //Position and scale character health
	//SDL_Rect dstRect2 = { posx2,posy2, w2, h2 }; //Position and scale character Amo

	switch (vidap)
	{
	case 1:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		break;
	case 2:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		break;
	case 3:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		break;
	case 4:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		break;
	case 5:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect5);//render  character health
		break;
	}
}

void UI::Boss1()
{
	//SDL_Rect dstRect2 = { posxb,posyb, wb, hb }; //Position and scale Boss health
	//if(figth){/*boss = Engine::GetInstance().textures->Load("Assets/Textures/UI "); //Load life of the player*/}
	SDL_Rect dstRect = { bposx,posy2, wb, hb}; //Position and scale character health
	SDL_Rect dstRect2 = { bposx2,posy2, wb, hb }; //Position and scale character health
	SDL_Rect dstRect3 = { bposx3,posy2, wb, hb }; //Position and scale character health
	SDL_Rect dstRect4 = { bposx4,posy2, wb, hb }; //Position and scale character health
	SDL_Rect dstRect5 = { bposx5,posy2, wb, hb }; //Position and scale character health
	SDL_Rect dstRect6 = { bposx6,posy2, wb, hb }; //Position and scale character health
	switch (vidab)
	{
	case 1:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect6);//render  character health
		break;
	case 2:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect6);//render  character health
		break;
	case 3:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect6);//render  character health
		break;
	case 4:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect5);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect6);//render  character health
		break;
	case 5:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect5);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida2, nullptr, &dstRect6);//render  character health
		break;
	case 6:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect5);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vida2, nullptr, &dstRect6);//render  character health
		break;
	}
	
}


