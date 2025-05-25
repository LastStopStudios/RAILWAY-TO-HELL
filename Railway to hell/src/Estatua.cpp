#include "Estatua.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Estatua::Estatua() : Entity(EntityType::ESTATUA)
{

}

Estatua::~Estatua() {}

bool Estatua::Awake() {
	
	return true;
}

bool Estatua::Start() {
	
	return true;
}

bool Doors::Update(float dt){

	return true;
}

bool Doors::CleanUp()
{

	return true;
}
