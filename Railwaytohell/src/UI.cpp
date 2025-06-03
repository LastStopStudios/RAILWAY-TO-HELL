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

	return true;
}

bool UI::PostUpdate()
{
	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
		Engine::GetInstance().scene->ResetSkipInput();
		return true;
	}

	renderUI();

	if (PopeadaTime) { PopUps(); }
	return true;
}

bool UI::CleanUp()
{
	if (vidaB1 != nullptr) {
		Engine::GetInstance().textures->UnLoad(vidaB1);//Unload text background
		vidaB1 = nullptr;
	}
	if (vidaB2 != nullptr) {
		Engine::GetInstance().textures->UnLoad(vidaB2);//Unload text background
		vidaB2 = nullptr;
	}
	if (vidaB3 != nullptr) {
		Engine::GetInstance().textures->UnLoad(vidaB3);//Unload text background
		vidaB3 = nullptr;
	}
	if (vidaB4 != nullptr) {
		Engine::GetInstance().textures->UnLoad(vidaB4);//Unload text background
		vidaB4 = nullptr;
	}
	if (Evida != nullptr) {
		Engine::GetInstance().textures->UnLoad(Evida);//Unload text background
		Evida = nullptr;
	}
	if (vidapl != nullptr) {
		Engine::GetInstance().textures->UnLoad(vidapl);//Unload text background
		vidapl = nullptr;
	}
	if (vidapl2 != nullptr) {
		Engine::GetInstance().textures->UnLoad(vidapl2);//Unload text background
		vidapl2 = nullptr;
	}
	return true;
}

void UI::LoadTUi()
{
	//Player
	vidapl = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Protagonist_Health_Heart_Empty.png"); //Load life of the player
	vidapl2 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Protagonist_Health_Heart_Full.png"); //Load Empty life of the player

	//Bosses
	vidaB1 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Boss_Health_Heart_Noma_Full.png"); //Load boss 1 life 
	vidaB2 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Boss_Health_Heart_Asmodeos_Full.png"); //Load boss 2 life 
	vidaB3 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Boss_Health_Heart_Satanas_Full.png"); //Load boss 3 life 
	vidaB4 = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Boss_Health_Heart_Satanas_Full_V2.png"); //Load boss 4 life 
	Evida = Engine::GetInstance().textures->Load("Assets/Textures/UI/Bar_Boss_Health_Heart_Empty.png"); //Load Empty boss life 

	//amo = Engine::GetInstance().textures->Load("Assets/Textures/UI "); //Load stamina of the player
		
	//PopUps
	Ball = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/ball_teclat.png"); //Load ball keyboard PopUp texture
	Ball2 = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/ball_xboxt.png"); //Load ball Controller PopUp texture
	Dash = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/dash_teclat.png"); //Load Dash keyboard PopUp texture
	Dash2 = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/dash_xbox.png"); //Load Dash Controller PopUp texture
	DJump = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/doublejump_teclat.png"); //Load Double jump keyboard PopUp texture
	DJump2 = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/doublejump_xbox.png"); //Load Double jump Controller PopUp texture
	Whip = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/whip_teclat.png"); //Load Whip keyboard PopUp texture
	Whip2 = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/whip_xbox.png"); //Load Whip Controller PopUp texture
	Puzzle = Engine::GetInstance().textures->Load("Assets/Textures/PopUps/puzzle.png"); //Load Puzzle complete PopUp texture
}

void UI::renderUI()
{
	PJs();
	if (figth) { Boss1();}
	if (figth2) { Boss2(); }
	if (figth3) { Boss3(); }
}
void UI::PopUps() {

	SDL_Rect PopRect = { 0,0, 1300, 800 }; //Position and scale Pop Up
	Engine::GetInstance().entityManager->DialogoOn();//stop entities



	if (Engine::GetInstance().IsControllerConnected()) {//controller input
		switch (item)
		{
		case 1:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Ball2, nullptr, &PopRect);
			break;
		case 2:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Dash2, nullptr, &PopRect);
			break;
		case 3:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, DJump2, nullptr, &PopRect);
			break;
		case 4:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Whip2, nullptr, &PopRect);
			break;
		case 5:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Puzzle, nullptr, &PopRect);
			break;
		}
	}
	else {//Keyboard input
		switch (item)
		{
		case 1:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Ball, nullptr, &PopRect);
			break;
		case 2:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Dash, nullptr, &PopRect);
			break;
		case 3:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, DJump, nullptr, &PopRect);
			break;
		case 4:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Whip, nullptr, &PopRect);
			break;
		case 5:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Puzzle, nullptr, &PopRect);
			break;
		}
	}

	if (PopeadaTime) {
		//End PopUp
	// Check controller input
		if (Engine::GetInstance().IsControllerConnected()) {
			SDL_GameController* controller = Engine::GetInstance().GetGameController();
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) == KEY_DOWN) {
				Engine::GetInstance().entityManager->DialogoOff();
				PopeadaTime = false;
			}
		}
		// keyboard input
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			Engine::GetInstance().entityManager->DialogoOff();
			PopeadaTime = false;
		}
	}

	
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
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect5);//render  character health
		break;
	case 2:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect5);//render  character health
		break;
	case 3:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect5);//render  character health
		break;
	case 4:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl2, nullptr, &dstRect5);//render  character health
		break;
	case 5:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect2);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect3);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect4);//render  character health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidapl, nullptr, &dstRect5);//render  character health
		break;
	}
}

void UI::Boss1()
{
	SDL_Rect dstRect = { bposx,posy2, wb, hb}; //Position and scale Boss health
	SDL_Rect dstRect2 = { bposx2,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect3 = { bposx3,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect4 = { bposx4,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect5 = { bposx5,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect6 = { bposx6,posy2, wb, hb }; //Position and scale Boss health
	switch (vidab1)
	{
	case 1:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		break;
	case 2:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		break;
	case 3:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		break;
	case 4:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		break;
	case 5:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		break;
	case 6:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB1, nullptr, &dstRect6);//render  Boss health
		break;
	}
	
}
void UI::Boss2()
{
	SDL_Rect dstRect = { bposx,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect2 = { bposx2,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect3 = { bposx3,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect4 = { bposx4,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect5 = { bposx5,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect6 = { bposx6,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect7 = { bposx7,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect8 = { bposx8,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect9 = { bposx9,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect10 = { bposx10,posy2, wb, hb }; //Position and scale Boss health
	switch (vidab2)
	{
	case 1:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 2:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 3:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 4:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 5:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 6:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 7:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 8:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 9:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect10);//render  Boss health
		break;
	case 10:
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect2);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect3);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect4);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect5);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect6);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect7);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect8);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect9);//render  Boss health
		SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB2, nullptr, &dstRect10);//render  Boss health
		break;
	}

}
void UI::Boss3()
{
	SDL_Rect dstRect = { bposx,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect2 = { bposx2,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect3 = { bposx3,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect4 = { bposx4,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect5 = { bposx5,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect6 = { bposx6,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect dstRect7 = { bposx7,posy2, wb, hb }; //Position and scale Boss health
	SDL_Rect vidaphase1 = { 610,posy2, wb, hb }; //Position and scale Boss health
	if(fase1){
		switch (vidab3)
		{
		case 0:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &vidaphase1);//render  Boss health
			break;
		case 1:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &vidaphase1);//render  Boss health
			break;
		}
	}
	if (fase2) {
		switch (vidab3)
		{
		case 0:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			break;
		case 1:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			break;
		case 2:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			break;
		case 3:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			break;
		case 4:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			break;
		}
	}
	if (fase3) {
		switch (vidab3)
		{
		case 1:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
			break;
		case 2:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
			break;
		case 3:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
			break;
		case 4:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
			break;
		case 5:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect6);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
			break;
		case 6:
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect2);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect3);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect4);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect5);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, vidaB3, nullptr, &dstRect6);//render  Boss health
			SDL_RenderCopy(Engine::GetInstance().render->renderer, Evida, nullptr, &dstRect7);//render  Boss health
			break;
		}
	}
	

}


