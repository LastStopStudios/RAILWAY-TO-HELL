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

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	Type = parameters.attribute("name").as_string();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));

	currentAnimation = &idle;

	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW / 2, texH, bodyType::KINEMATIC);

	// Assign collider type
	if (pbody != nullptr) {
		pbody->listener = this;
		pbody->ctype = ColliderType::ESTATUAS;
	}
	
	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Estatua::Update(float dt){

	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	if (pbody == nullptr) {
		LOG("Enemy pbody is null!");
		return false;
	}

	if (GetType() == "estatua1") {
		if (Engine::GetInstance().entityManager->estatua1){currentAnimation = &activated;}
	}

	if (GetType() == "estatua2") {
		if (Engine::GetInstance().entityManager->estatua2) { currentAnimation = &activated; }
	}

	return true;
}

bool Estatua::CleanUp()
{

	return true;
}
