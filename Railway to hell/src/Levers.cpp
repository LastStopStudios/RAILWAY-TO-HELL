#include "Levers.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Physics.h"

Levers::Levers() : Entity(EntityType::LEVER)
{

}

Levers::~Levers() {}

bool Levers::Awake() {
	return true;
}

bool Levers::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	leverType = parameters.attribute("name").as_string();


	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	lever_activated.LoadAnimations(parameters.child("animations").child("lever_activated"));
	currentAnimation = &idle;

	// Add a physics to an door - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW / 2, texH, bodyType::KINEMATIC);

	pbody->listener = this;

	//Assign collider type
	pbody->ctype = ColliderType::LEVER;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Levers::Update(float dt)
{
	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	// Add a physics to an enemy - update the position of the object from the physics.  
	if (pbody == nullptr) {
		LOG("Enemy pbody is null!");
		return false;
	}


	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (GetLeverType() == "lever") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());

	}
	if (GetLeverType() == "lever_memory_left") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());

	}
	if (GetLeverType() == "lever_to_Station_E1L1") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());

	}
	currentAnimation->Update();



	return true;
}

void Levers::OnCollision(PhysBody* physA, PhysBody* physB) {
	if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::ENEMY) {
		// Additional enemy hit logic can go here
		return;
	}
	switch (physB->ctype) {
	case ColliderType::PLATFORM:
		break;
	case ColliderType::PLAYER: 
		break;
	case ColliderType::PLAYER_WHIP_ATTACK: {
		if (GetLeverType() == "lever") {
			if (!Activated) {
				Lever_Door_Activated = true;
				currentAnimation = &lever_activated;
			}
		}
		if (GetLeverType() == "lever_memory_left") {
			if (!Activated) {
				Lever_Door_Activated = true;
				currentAnimation = &lever_activated;
			}
		}
		if (GetLeverType() == "lever_to_Station_E1L1") {
			if (!Activated) {
				Lever_Door_Activated = true;
				currentAnimation = &lever_activated;
			}
		}
		break;
	}
	case ColliderType::UNKNOWN:
		break;
	}
}

void Levers::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

}

bool Levers::CleanUp()
{


	if (pbody != nullptr) {
		pbody->listener = nullptr;
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}


	return true;
}