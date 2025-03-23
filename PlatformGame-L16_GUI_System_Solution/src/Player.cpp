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

	//L03: TODO 2: Initialize Player parameters
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

	meleeAttack = Animation();
	meleeAttack.speed = 0.15f;
	meleeAttack.loop = false;

	// Load attack texture
	attackTexture = Engine::GetInstance().textures.get()->Load("Assets/Cuerpo_a_Cuerpo.png");
	// Set initial state
	isAttacking = false;
	canAttack = true;
	attackCooldown = 0.0f;
	attackHitbox = nullptr;
	facingRight = true;

	return true;
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

	if (!parameters.attribute("gravity").as_bool()) {
		velocity = b2Vec2(0, 0);
	}

	// Move left
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -0.2 * 16;
		facingRight = false;
	}

	// Move right
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = 0.2 * 16;
		facingRight = true;
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
	if (isJumping == true)
	{
		velocity.y = pbody->body->GetLinearVelocity().y;
	}

	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
	int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1)
	{
		Engine::GetInstance().sceneLoader->LoadScene(1);
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2)
	{
		Engine::GetInstance().sceneLoader->LoadScene(2);
	}

	UpdateMeleeAttack(dt);
	DrawPlayer();

	/*Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();*/
	return true;
}

void Player::UpdateMeleeAttack(float dt) {
	// Actualizar cooldown si no puede atacar
	if (!canAttack) {
		attackCooldown -= dt;
		if (attackCooldown <= 0.0f) {
			canAttack = true;
			attackCooldown = 0.0f;
		}
	}

	// Iniciar ataque si se presiona la tecla, no est� atacando y puede atacar
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN && !isAttacking && canAttack) {
		LOG("Iniciando nuevo ataque");
		isAttacking = true;
		canAttack = false;
		attackCooldown = 0.5f;

		// Reiniciar animaci�n
		meleeAttack = Animation();
		meleeAttack.speed = 0.15f;
		meleeAttack.loop = false;

		// A�adir los frames de la animaci�n nuevamente
		for (int i = 0; i < 7; i++) {
			SDL_Rect rect = { i * 64, 0, 64, 64 };
			meleeAttack.PushBack(rect);
		}

		// Offset para mover la hitbox un poco m�s a la derecha
		int attackOffset = 20;
		int attackX = facingRight ? position.getX() + texW + attackOffset : position.getX() - 64 - attackOffset;

		// Eliminar hitbox anterior si existe
		if (attackHitbox) {
			Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
			attackHitbox = nullptr;
		}

		// Crear nueva hitbox
		attackHitbox = Engine::GetInstance().physics.get()->CreateRectangleSensor(
			attackX, position.getY(), 64, 64, bodyType::DYNAMIC);
		attackHitbox->ctype = ColliderType::PLAYER_ATTACK;
		attackHitbox->listener = this;
	}

	// Actualizar la animaci�n si estamos atacando
	if (isAttacking) {
		meleeAttack.Update();

		LOG("Frame de animaci�n: %d, HasFinished: %d",
			meleeAttack.GetCurrentFrame().x / 64, meleeAttack.HasFinished());

		// Mover hitbox con el jugador
		if (attackHitbox) {
			int attackOffset = 20;
			int attackX = facingRight ? position.getX() + texW + attackOffset : position.getX() - 64 - attackOffset;
			attackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(position.getY()) }, 0);
		}

		// Terminar ataque cuando acabe la animaci�n
		if (meleeAttack.HasFinished()) {
			LOG("Animaci�n finalizada, terminando ataque");
			isAttacking = false;

			if (attackHitbox) {
				Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
				attackHitbox = nullptr;
			}
		}
	}
}

void Player::DrawPlayer() {
	if (isAttacking) {
		SDL_Rect currentFrame = meleeAttack.GetCurrentFrame();
		// Calcula la posici�n de ataque en funci�n de la direcci�n
		int attackX = facingRight ? position.getX() + texW : position.getX() - (64 + texW);
		SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

		Engine::GetInstance().render.get()->DrawTexture(
			attackTexture, attackX, position.getY() - 30, &currentFrame, 0.0, 0.0, flip);
	}
	else {
		SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

		Engine::GetInstance().render.get()->DrawTexture(
			texture, (int)position.getX(), (int)position.getY(),
			&currentAnimation->GetCurrentFrame(), 0.0, 0.0, flip);

		currentAnimation->Update();
	}
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		isJumping = false;
		break;
	case ColliderType::ITEM:
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
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
	return pos;
}