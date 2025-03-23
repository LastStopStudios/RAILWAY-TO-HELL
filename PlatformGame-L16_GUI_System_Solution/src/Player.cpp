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
    // Update attack cooldown timer if player is currently unable to attack
    if (!canAttack) {
        attackCooldown -= dt;
        if (attackCooldown <= 0.0f) {
            canAttack = true;
            attackCooldown = 0.0f;
        }
    }

    // Initiate attack when H key is pressed, if player is not already attacking and cooldown has expired
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN && !isAttacking && canAttack) {
        // Set attack state flags
        isAttacking = true;
        canAttack = false;
        attackCooldown = 0.5f;  // Set cooldown duration (0.5 seconds)

        // Initialize a new attack animation
        meleeAttack = Animation();
        meleeAttack.speed = 0.15f;  // Animation playback speed
        meleeAttack.loop = false;   // Animation should not loop

        // Load all frames for the attack animation (7 frames of 64x64 pixels each)
        for (int i = 0; i < 7; i++) {
            SDL_Rect rect = { i * 64, 0, 64, 64 };
            meleeAttack.PushBack(rect);
        }

        // Calculate the initial position for the attack hitbox
        int attackX = facingRight ? position.getX() : position.getX();

        // Remove any existing attack hitbox before creating a new one
        if (attackHitbox) {
            Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
            attackHitbox = nullptr;
        }

        // Create a new physics hitbox for the attack
        attackHitbox = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            attackX, position.getY(), 64, 64, bodyType::DYNAMIC);
        attackHitbox->ctype = ColliderType::PLAYER_ATTACK;  // Set collision type
        attackHitbox->listener = this;  // Register collision listener
    }

    // Handle ongoing attack state
    if (isAttacking) {
        // Update the attack animation
        meleeAttack.Update();

        // Update the position of the attack hitbox to follow the player
        if (attackHitbox) {
            // Position the hitbox in front of the player based on facing direction
            int attackX = facingRight ? position.getX() + 30 : position.getX();
            attackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(position.getY()) }, 0);
        }

        // End the attack when the animation finishes playing
        if (meleeAttack.HasFinished()) {
            isAttacking = false;

            // Clean up the attack hitbox
            if (attackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
                attackHitbox = nullptr;
            }
        }
    }
}

void Player::DrawPlayer() {
    // Determine the flip direction based on which way the player is facing
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Draw the player's normal sprite when not attacking
    if (!isAttacking) {
        // Use the new DrawTextureWithFlip function to render the player with proper orientation
        Engine::GetInstance().render.get()->DrawTextureWithFlip(
            texture,                                // Player texture
            (int)position.getX(),                   // X position
            (int)position.getY(),                   // Y position
            &currentAnimation->GetCurrentFrame(),   // Current animation frame
            0.0,                                    // Speed modifier (0.0 = no parallax effect)
            0,                                      // Rotation angle (0 = no rotation)
            INT_MAX, INT_MAX,                       // No pivot point (use default)
            flip);                                  // Horizontal flip based on direction

        // Advance the animation to the next frame
        currentAnimation->Update();
    }
    // Draw the attack animation when the player is attacking
    else {
        // Get the current frame of the attack animation
        SDL_Rect currentFrame = meleeAttack.GetCurrentFrame();
        int attackX;

        // Calculate the position for the attack animation based on facing direction
        if (facingRight) {
            attackX = position.getX();              // Position when facing right
        }
        else {
            attackX = position.getX() - 20;         // Offset position when facing left
        }

        // Draw the attack animation with proper positioning and orientation
        Engine::GetInstance().render.get()->DrawTextureWithFlip(
            attackTexture,                          // Attack animation texture
            attackX,                                // X position with direction offset
            position.getY() - 30,                   // Y position with vertical offset
            &currentFrame,                          // Current attack animation frame
            0.0,                                    // Speed modifier (0.0 = no parallax effect)
            0,                                      // Rotation angle (0 = no rotation)
            INT_MAX, INT_MAX,                       // No pivot point (use default)
            flip);                                  // Horizontal flip based on direction
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