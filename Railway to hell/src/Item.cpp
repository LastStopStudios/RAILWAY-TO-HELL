#include "Item.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Physics.h"

Item::Item() : Entity(EntityType::ITEM)
{

}

Item::~Item() {}

bool Item::Awake() {
	return true;
}

bool Item::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	itemType = parameters.attribute("name").as_string();


	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, texH / 2, bodyType::DYNAMIC);

	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::ITEM;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Item::Update(float dt)
{
	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	if (pbody == nullptr) {
		LOG("Enemy pbody is null!");
		return false;
	}
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

void Item::OnCollision(PhysBody* physA, PhysBody* physB) {
	if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::ENEMY) {
		LOG("Player attack hit an enemy!");
		// Additional enemy hit logic can go here
		return;
	}
	switch (physB->ctype) {
	case ColliderType::PLATFORM:
		break;
	case ColliderType::PLAYER: {
		if (GetItemType() == "Dash ability") {
			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Whip") {
			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		if (GetItemType() == "Door key") {
			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}

		break;
	}
	case ColliderType::UNKNOWN:
		break;
	}
}

void Item::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {

}

void Item::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

bool Item::CleanUp()
{


	Engine::GetInstance().physics->DeletePhysBody(pbody);

	return true;
}