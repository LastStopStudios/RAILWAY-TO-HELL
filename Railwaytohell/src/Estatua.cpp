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

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	activated.LoadAnimations(parameters.child("animations").child("activated"));

	currentAnimation = &idle;

	pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX() + texH , (int)position.getY() + texH / 2, 20, 20, bodyType::KINEMATIC);

	pbody->listener = this;
	pbody->ctype = ColliderType::ESTATUAS;
	
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
		return false;
	}

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (Engine::GetInstance().entityManager->estatua2){
		currentAnimation = &activated;
	}

	

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());

	currentAnimation->Update();

	return true;
}

void Estatua::OnCollision(PhysBody* physA, PhysBody* physB) {

}

void Estatua::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

}

bool Estatua::CleanUp()
{

	return true;
}
