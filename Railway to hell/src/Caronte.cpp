#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Caronte.h"

Caronte::Caronte() : Entity(EntityType::CARONTE)
{
}

Caronte::~Caronte() {}

bool Caronte::Awake() {
	return true;
}

bool Caronte::Start() {

	// Initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	// Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	attack.LoadAnimations(parameters.child("animations").child("attack"));
	hurt.LoadAnimations(parameters.child("animations").child("hurt"));
	die.LoadAnimations(parameters.child("animations").child("die"));
	currentAnimation = &idle;

	// Initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW, texH, bodyType::DYNAMIC);
	AttackSensorArea = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)position.getY(), texW * 2, texH * 1.2, bodyType::DYNAMIC);

	pbody->listener = this;
	AttackSensorArea->listener = this;

	// Assign collider type
	pbody->ctype = ColliderType::CARONTE;
	AttackSensorArea->ctype = ColliderType::ATTACKSENSOR;

	// Fix the rotation of the body
	pbody->body->SetFixedRotation(true);
	AttackSensorArea->body->SetFixedRotation(true);

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Caronte::Update(float dt)
{
	// wait for the first input to start the game
	if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
	{
		return true;
	}

	if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
		Engine::GetInstance().scene->ResetSkipInput();
		return true;
	}

	if (hurt.HasFinished()) {
		currentAnimation = &die;
	}

	if (isattacking) {

		if (!attacked) {
			attacked = true;
			canAttack = false;
			currentAttackCooldown = attackCooldown;
			currentAnimation = &attack;
			currentAnimation->Reset();

			if (AttackArea == nullptr) { // create the attack area
				AttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
					(int)position.getX() - 10,
					(int)position.getY() + texH / 2,
					70,
					40,
					bodyType::KINEMATIC
				);
				AttackArea->ctype = ColliderType::BOSS_ATTACK;
			}

		}


	}
	if (attack.HasFinished()) { // stop attacking
		candie = true;
		currentAnimation = &idle;
		attack.Reset();


		if (AttackArea != nullptr) { // delete the attack area
			Engine::GetInstance().physics.get()->DeletePhysBody(AttackArea);
			AttackArea = nullptr;
		}
	}


	// update the position of caronte from the physics  
	if (pbody == nullptr) {
		LOG("Caronte pbody is null!");
		return false;
	}
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Update the attack area position of caronte
	if (AttackSensorArea != nullptr) {
		int sensorX = position.getX() + texW;
		int sensorY = position.getY() + texH * 0.4;
		AttackSensorArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(sensorX), PIXEL_TO_METERS(sensorY)), 0);
	}

	SDL_Rect frame = currentAnimation->GetCurrentFrame();

	// draw the caronte
	if(currentAnimation == &idle){
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - (frame.w - texW * 1.3), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
	}
	if (currentAnimation == &attack) {
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW/2, (int)position.getY() - texH/3, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
	}
	if (currentAnimation == &hurt) {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - (frame.w - texW * 1.6), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
	}
	if (currentAnimation == &die) {
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW / 2, (int)position.getY() , &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
	}
	// update the animation
	currentAnimation->Update();



	return true;
}

void Caronte::OnCollision(PhysBody* physA, PhysBody* physB) {

	//if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::ENEMY) {
	//    LOG("Player attack hit an enemy!");
	//    return;
	//}

	switch (physB->ctype) {
	case ColliderType::PLAYER:
		if (!isattacking) {
			isattacking = true;
			LOG("Attack Player!");
		}
		if (die.HasFinished()) {
			Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
			item->SetParameters(Engine::GetInstance().scene.get()->itemConfigNode);
			Engine::GetInstance().scene.get()->itemList.push_back(item);
			item->Start();
			Vector2D pos(position.getX() + texW, position.getY());
			item->SetPosition(pos);

			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		break;
	case ColliderType::PLAYER_ATTACK:
		if (candie) {
		candie = false;
		currentAnimation = &hurt;
		}



		break;
	case ColliderType::ITEM:
		break;
	case ColliderType::SENSOR:
		break;
	case ColliderType::ASCENSORES:
		break;
	case ColliderType::DIALOGOS:
		break;

	case ColliderType::UNKNOWN:
		break;
	}
}

void Caronte::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	// at the moment is not being used
}

//Vector2D Caronte::GetPosition() {
//	b2Vec2 bodyPos = pbody->body->GetTransform().p;
//	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
//	return pos;
//}

bool Caronte::CleanUp()
{

	Engine::GetInstance().physics->DeletePhysBody(AttackSensorArea);
	Engine::GetInstance().physics->DeletePhysBody(pbody);

	return true;
}