#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "SceneLoader.h"
#include "Map.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	return true;
}

bool Player::Start() {

	//Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");

	return true;
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

	if (!parameters.attribute("gravity").as_bool()) {
		velocity = b2Vec2(0,0);
	}

	// Move left
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -0.2 * 16;
	}

	// Move right
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = 0.2 * 16;
	}

	// Move Up
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		velocity.y = -0.2 * 16;
	}

	// Move down
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
		velocity.y = 0.2 * 16;
	}
	
	//Jump
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false) {
		// Apply an initial upward force
		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
		isJumping = true;
	}

	// If the player is jumpling, we don't want to apply gravity, we use the current velocity prduced by the jump
	if(isJumping == true)
	{
		velocity.y = pbody->body->GetLinearVelocity().y;
	}

	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentScene();
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1)
	{
		Engine::GetInstance().sceneLoader->LoadScene(1);
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2)
	{
		Engine::GetInstance().sceneLoader->LoadScene(2);
	}

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();
	return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	sensorId = 0;
	Engine::GetInstance().textures.get()->UnLoad(texture);
	esc.clear();
	return true;
}

// Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentScene();
	
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		isJumping = false;
		break;
	case ColliderType::ITEM:
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
		break;
	case ColliderType::SENSOR:
		esc.clear();
		LOG("!!!!TOCO EL SENSOR!!!!");
		valorid = SacarId();
		LOG("!!!!valorid en collider sensor despues SacarId: %d!!!!", valorid);
		esc = SacarValor(valorid);; // Acceder al valor de tipo string
		LOG("!!!!!!!!!!debajo del esc: %s!!!!!!!!!!!!!", esc.c_str());
		if (esc == "S1S2") {
			Engine::GetInstance().sceneLoader->LoadScene(2);
		}else if (esc == "S2S1") {
			Engine::GetInstance().sceneLoader->LoadScene(1);
		}


		break;
	case ColliderType::UNKNOWN:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		break;
	case ColliderType::ITEM:
		break;
	case ColliderType::UNKNOWN:
		break;
	}
}

void Player::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos,0);
}

Vector2D Player::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	LOG("Posición Player: x=%.2f, y=%.2f", pos.getX(), pos.getY());
	return pos;
}


int Player :: SacarId() {
	LOG("!!!!Entro en SacarID!!!!");
	Vector2D posicion = GetPosition();//sacar posicion del player
	LOG("!!!!Saco posicion player X en SacarID, x=%.2f !!!!", posicion.getX());
	LOG("!!!!Saco posicion player Y en SacarID, y=%.2f !!!!", posicion.getY());
	sensorId = Engine::GetInstance().map->GetSensorId(posicion.getX(), posicion.getY());// ver ID del sensor que toca el player	
	LOG("!!!!Id del sensor con la posicion del player: %d!!!!", sensorId);
	return sensorId;
}

std::string Player:: SacarValor(int ids) {

	LOG("!!!!Id del sensor con la posicion del player desde SacarValor: %s!!!!", ids);
	std::string valor = Engine::GetInstance().map->ValorPorId(ids);//Buscar el valor usando la id
	LOG("!!!!Id del sensor con la posicion del player: %d!!!!", valor);
	return valor;
	
}