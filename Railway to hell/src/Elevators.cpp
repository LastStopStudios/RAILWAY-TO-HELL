#include "Elevators.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Physics.h"

Elevators::Elevators() : Entity(EntityType::ELEVATORS)
{

}

Elevators::~Elevators() {}

bool Elevators::Awake() {
	return true;
}

bool Elevators::Start() {
	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));
	currentAnimation = &idle;


	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texW /2 , (int)position.getY() + texH / 2, texW, texH, bodyType::KINEMATIC);

	pbody->listener = this;
	pbody->ctype = ColliderType::ASCENSOR;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

// Called each loop iteration
bool Elevators::PreUpdate()
{

	return true;
}


bool Elevators::Update(float dt){
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
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());

	currentAnimation->Update();

	return true;
}

void Elevators::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
		case ColliderType::PLAYER:
			if(Engine::GetInstance().entityManager->Ascensor == true){
				currentAnimation = &activated;
			}
			break;
		case ColliderType::UNKNOWN:
			break;
	}

}

void Elevators::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		LOG("Toco Dibujo");
		currentAnimation = &idle;
		break;
	case ColliderType::UNKNOWN:
		break;
	}
}

bool Elevators::CleanUp()
{
	if (pbody != nullptr) {
		pbody->listener = nullptr;
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}

	return true;
}