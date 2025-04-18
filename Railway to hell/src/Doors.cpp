#include "Doors.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Physics.h"

Doors::Doors() : Entity(EntityType::DOORS)
{

}

Doors::~Doors() {}

bool Doors::Awake() {
	return true;
}

bool Doors::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	doorType = parameters.attribute("name").as_string();


	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));
	lever_door_activated.LoadAnimations(parameters.child("animations").child("lever_door_activated"));
	lever_door_Viewleft.LoadAnimations(parameters.child("animations").child("lever_door_Viewleft"));
	door_lever_memory_left_activated.LoadAnimations(parameters.child("animations").child("door_lever_memory_left_activated"));
	door_lever_to_station_activated.LoadAnimations(parameters.child("animations").child("door_lever_to_station_activated"));
	door_lever_next_to_dashItem_activated.LoadAnimations(parameters.child("animations").child("door_lever_next_to_dashItem_activated"));
	currentAnimation = &idle;

	// Add a physics to an door - initialize the physics body
	if (GetDoorType() == "whip boss door") {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW / 2, texH, bodyType::KINEMATIC);
		pbody2 = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, 160, 112, bodyType::KINEMATIC);
		pbody2->listener = this;
	}
	else if (GetDoorType() == "lever_door") {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);
	}
	else if (GetDoorType() == "door_lever_2") {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);
	}
	else if (GetDoorType() == "door_lever_memory_left") {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);
	}
	else if (GetDoorType() == "door_lever_to_station") {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);
	}
	else if (GetDoorType() == "door_lever_next_to_dashItem") {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);
	}
	else {
		pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);
	}

	// Assign collider type
	if (pbody != nullptr) {
		pbody->listener = this;
		pbody->ctype = ColliderType::DOORS;
	}

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Doors::Update(float dt)
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
	//if (GetDoorType() == "lever_door" ) {
	//	if (Engine::GetInstance().scene.get()->GetPlayer()->returnLeverOne()) {
	//		currentAnimation = &lever_door_activated;
	//	}
	//}
	if (GetDoorType() == "door_lever_2") {
		if (Engine::GetInstance().scene.get()->GetPlayer()->returnLeverOne()) {
			currentAnimation = &lever_door_Viewleft;
		}
	}
	if (GetDoorType() == "door_lever_memory_left") {
		if (Engine::GetInstance().scene.get()->GetPlayer()->returnLeverTwo()) {
			currentAnimation = &door_lever_memory_left_activated;
		}
	}
	if (GetDoorType() == "door_lever_to_station") {
		if (Engine::GetInstance().scene.get()->GetPlayer()->returnLeverThree()) {
			currentAnimation = &door_lever_to_station_activated;
		}
	}
	if (GetDoorType() == "door_lever_next_to_dashItem") {
		if (Engine::GetInstance().scene.get()->GetPlayer()->returnLeverFour()) {
			currentAnimation = &door_lever_next_to_dashItem_activated;
		}
	}

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (GetDoorType() == "whip boss door") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW, (int)position.getY(), &currentAnimation->GetCurrentFrame()); 
	}
	if (GetDoorType() == "lever_door") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW/2, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}
	if (GetDoorType() == "door_lever_2") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 2, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}
	if (GetDoorType() == "door_lever_memory_left") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 2, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}
	if (GetDoorType() == "door_lever_to_station") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 2, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}
	if (GetDoorType() == "door_lever_next_to_dashItem") {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 2, (int)position.getY(), &currentAnimation->GetCurrentFrame());
	}
	currentAnimation->Update();

	return true;
}

void Doors::OnCollision(PhysBody* physA, PhysBody* physB) {
	if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::ENEMY) {
		// Additional enemy hit logic can go here
		return;
	}
	switch (physB->ctype) {
	case ColliderType::PLATFORM:
		break;
	case ColliderType::PLAYER: {
		
		if (GetDoorType() == "whip boss door") {
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
				if (!Activated && Engine::GetInstance().scene.get()->GetPlayer()->returnCanOpenDoor()) {
					Activated = true;
					currentAnimation = &activated;
				}
			}
		}
		
		//if (GetDoorType() == "lever") {
		//	if (!Activated) {
		//		Activated = true;
		//		currentAnimation = &lever_activated;
		//	}
		//}
		if (GetDoorType() == "door_lever_2") {
			if (lever_door_Viewleft.HasFinished()) {
				Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			}
		}
		if (GetDoorType() == "door_lever_memory_left") {
			if (door_lever_memory_left_activated.HasFinished()) {
				Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			}
		}
		if (GetDoorType() == "door_lever_to_station") {
			if (door_lever_to_station_activated.HasFinished()) {
				Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			}
		}
		if (GetDoorType() == "door_lever_next_to_dashItem") {
			if (door_lever_next_to_dashItem_activated.HasFinished()) {
				Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			}
		}
		if (GetDoorType() == "lever_door") {
			if (lever_door_activated.HasFinished()) {
				Engine::GetInstance().entityManager.get()->DestroyEntity(this);
			}
		}

		if (activated.HasFinished()) {
			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		break;
	}
	case ColliderType::UNKNOWN:
		break;
	}
}

void Doors::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

}

bool Doors::CleanUp()
{

	if (GetDoorType() == "lever_door") {
		if (pbody != nullptr) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
	}
	if (GetDoorType() == "door_lever_2") {
		if (pbody != nullptr) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
	}
	if (GetDoorType() == "door_lever_memory_left") {
		if (pbody != nullptr) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
	}
	if (GetDoorType() == "door_lever_to_station") {
		if (pbody != nullptr) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
	}
	if (GetDoorType() == "door_lever_next_to_dashItem") {
		if (pbody != nullptr) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
	}
	if (GetDoorType() == "whip boss door") {
		if (pbody != nullptr) {
			pbody->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
		}
		if (pbody2 != nullptr) {
			pbody2->listener = nullptr;
			Engine::GetInstance().physics->DeletePhysBody(pbody2);
			pbody2 = nullptr;
		}
	}



	return true;
}