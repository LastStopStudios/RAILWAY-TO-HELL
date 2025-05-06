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
	//if (dialogoOn) { return true; }
	

	return true;
}

bool UI::PostUpdate()
{
	
	return true;
}

bool UI::CleanUp()
{
	
	return true;
}