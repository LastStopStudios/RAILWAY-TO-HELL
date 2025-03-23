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
    // Initialize Player parameters
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    currentAnimation = &idle;

    // Add physics to the player - initialize physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) {
        pbody->body->SetGravityScale(0);
    }

    // Initialize audio effect
    pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");

    // Setup melee attack
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
    // Initialize velocity vector
    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    if (!parameters.attribute("gravity").as_bool()) {
        velocity = b2Vec2(0, 0);
    }

    // Movement controls
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        velocity.x = -0.2f * 16;
        facingRight = false;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        velocity.x = 0.2f * 16;
        facingRight = true;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
        velocity.y = -0.2f * 16;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
        velocity.y = 0.2f * 16;
    }

    // Jump control
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
        isJumping = true;
    }

    // If jumping, preserve the vertical velocity
    if (isJumping) {
        velocity.y = pbody->body->GetLinearVelocity().y;
    }

    // Apply the velocity to the player
    pbody->body->SetLinearVelocity(velocity);

    // Update position from physics body
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Level switching controls
    int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1) {
        Engine::GetInstance().sceneLoader->LoadScene(1);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2) {
        Engine::GetInstance().sceneLoader->LoadScene(2);
    }

    UpdateMeleeAttack(dt);
    DrawPlayer();

    return true;
}

void Player::UpdateMeleeAttack(float dt) {
    // Update cooldown if player can't attack
    if (!canAttack) {
        attackCooldown -= dt;
        if (attackCooldown <= 0.0f) {
            canAttack = true;
            attackCooldown = 0.0f;
        }
    }

    // Start attack if H key is pressed, player is not attacking and can attack
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN && !isAttacking && canAttack) {
        isAttacking = true;
        canAttack = false;
        attackCooldown = 0.5f;

        // Reset animation
        meleeAttack = Animation();
        meleeAttack.speed = 0.15f;
        meleeAttack.loop = false;

        // Add animation frames
        for (int i = 0; i < 7; i++) {
            SDL_Rect rect = { i * 64, 0, 64, 64 };
            meleeAttack.PushBack(rect);
        }

        // Calculate attack hitbox position
        int attackX = facingRight ? position.getX() : position.getX() ;

        // Delete previous hitbox if it exists
        if (attackHitbox) {
            Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
            attackHitbox = nullptr;
        }

        // Create new hitbox
        attackHitbox = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            attackX , position.getY(), 64, 64, bodyType::DYNAMIC);
        attackHitbox->ctype = ColliderType::PLAYER_ATTACK;
        attackHitbox->listener = this;
    }

    // Update attack animation and hitbox
    if (isAttacking) {
        meleeAttack.Update();

        // Update hitbox position to follow player
        if (attackHitbox) {
            int attackX = facingRight ? position.getX() + 30 : position.getX();
            attackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(position.getY()) }, 0);
        }

        // End attack when animation finishes
        if (meleeAttack.HasFinished()) {
            isAttacking = false;

            if (attackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
                attackHitbox = nullptr;
            }
        }
    }
}

void Player::DrawPlayer() {
    // Set flip based on direction
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Draw normal player sprite when not attacking
    if (!isAttacking) {
        Engine::GetInstance().render.get()->DrawTexture(
            texture, (int)position.getX(), (int)position.getY(),
            &currentAnimation->GetCurrentFrame(), 0.0, 0.0, flip);
        currentAnimation->Update();
    }
    // Draw attack animation when attacking
    else {
        SDL_Rect currentFrame = meleeAttack.GetCurrentFrame();
        int attackX;

        // IMPORTANTE: Estos valores deben ajustarse correctamente para el flip
        if (facingRight) {
            attackX = position.getX() ; // Ajustado a -20 para mejor posición
        }
        else {
            attackX = position.getX() ; // Valor clave para ajustar la posición cuando mira a la izquierda
        }

        // Draw attack animation with proper positioning and flip
        Engine::GetInstance().render.get()->DrawTexture(
            attackTexture, attackX, position.getY() - 30, &currentFrame, 0.0, 0.0, flip);
    }
}

bool Player::CleanUp() {
    LOG("Cleanup player");
    Engine::GetInstance().textures.get()->UnLoad(texture);
    return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isJumping = false;
        break;
    case ColliderType::ITEM:
        Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        Engine::GetInstance().physics.get()->DeletePhysBody(physB);
        break;
    case ColliderType::UNKNOWN:
        break;
    }
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    // Implementación vacía pero necesaria
}

void Player::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Player::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}