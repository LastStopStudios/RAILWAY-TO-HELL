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
#include "dialogoM.h"

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
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW/2, bodyType::DYNAMIC);
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
    meleeAttack.LoadAnimations(parameters.child("animations").child("attack"));
    pugi::xml_node attackNode = parameters.child("animations").child("attack");
    attackTexture = Engine::GetInstance().textures.get()->Load(attackNode.attribute("texture").as_string());

    // Load whip attack texture
    whipAttack.LoadAnimations(parameters.child("animations").child("whip"));
    pugi::xml_node whipNode = parameters.child("animations").child("whip");
    whipAttackTexture = Engine::GetInstance().textures.get()->Load(whipNode.attribute("texture").as_string());
    whipAttack = Animation();
    whipAttack.speed = 0.1f;
    whipAttack.loop = false;

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
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();  
        return true;
    }
    // Initialize velocity vector
    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    if (!parameters.attribute("gravity").as_bool()) {
        velocity = b2Vec2(0, 0);
    }

    // Mutually exclusive action handling
    if (!isAttacking && !isWhipAttacking && !isDashing) {
        HandleMovement(velocity);
        HandleJump();
    }

    // Handle dash only when not attacking or jumping
    if (!isAttacking && !isWhipAttacking && !isJumping) {
        HandleDash(velocity, dt);
    }

    // Handle attacks only when not dashing
    if (!isDashing) {
        UpdateWhipAttack(dt);
        UpdateMeleeAttack(dt);
    }

    // If dashing, preserve the dash velocity
    if (isDashing) {
        dashDuration -= dt;
        if (dashDuration <= 0) {
            isDashing = false;
        }
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

    DrawPlayer();
    HandleSceneSwitching();

    return true;
}

void Player::HandleMovement(b2Vec2& velocity) {
    // Horizontal movement
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        velocity.x = -0.2f * 16;
        facingRight = false;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        velocity.x = 0.2f * 16;
        facingRight = true;
    }

    // Vertical movement
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
        velocity.y = -0.2 * 16;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
        velocity.y = 0.2f * 16;
    }
}

void Player::HandleDash(b2Vec2& velocity, float dt) {
    // Dash cooldown logic
    if (!canDash) {
        dashCooldownTimer -= dt;
        if (dashCooldownTimer <= 0) {
            canDash = true;
        }
    }

    // Dash input with strict conditions
    bool isDashKeyPressed =
        (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT ||
            Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) &&
        Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN;

    if (isDashKeyPressed && canDash && Dash) {
        isDashing = true;
        dashDuration = 0.2f;  // Fixed dash duration

        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
            velocity.x = dashSpeed * 100;  // Dash right
            facingRight = true;
        }

        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
            velocity.x = -dashSpeed * 100;  // Dash left
            facingRight = false;
        }

        // Start the cooldown
        canDash = false;
        dashCooldownTimer = dashCooldownDuration;
    }
}

void Player::HandleJump() {
    // Jump control
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping) {
        pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
        isJumping = true;
    }
}

void Player::HandleSceneSwitching() {
    // Level switching controls
    int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1) {//pasar escena 1
        Engine::GetInstance().sceneLoader->LoadScene(1, 257, 527, false);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2) {//pasar escena 2
        Engine::GetInstance().sceneLoader->LoadScene(2, 100, 520, false);
    }
   /* if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_3) == KEY_DOWN && currentLvl != 2) {//pasar escena 3
        Engine::GetInstance().sceneLoader->LoadScene(sceneToLoad, Playerx, Playery, true);
    }*/
}

void Player::UpdateWhipAttack(float dt) {
    // Whip attack cooldown logic
    if (!canWhipAttack) {
        whipAttackCooldown -= dt;
        if (whipAttackCooldown <= 0.0f) {
            canWhipAttack = true;
            whipAttackCooldown = 0.0f;
        }
    }

    // Initiate whip attack only when not dashing, melee attacking, or already whip attacking
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN &&
        !isWhipAttacking && !isAttacking && !isDashing && canWhipAttack && WhipAttack) {
        // Existing whip attack initialization code remains the same
        isWhipAttacking = true;
        canWhipAttack = false;
        whipAttackCooldown = 0.7f;

        // Reset and load whip attack animation
        whipAttack = Animation();
        whipAttack.speed = 0.15f;
        whipAttack.loop = false;
        whipAttack.LoadAnimations(parameters.child("animations").child("whip"));

        // Create whip attack hitbox (existing code)
        int attackWidth = texW * 2;
        int attackHeight = texH / 2;
        b2Vec2 playerCenter = pbody->body->GetPosition();
        int centerX = METERS_TO_PIXELS(playerCenter.x);
        int centerY = METERS_TO_PIXELS(playerCenter.y);

        // Calculate attack position based on facing direction
        int attackX = facingRight ? centerX + texW : centerX - texW - attackWidth;

        // Remove existing whip attack hitbox
        if (whipAttackHitbox) {
            Engine::GetInstance().physics.get()->DeletePhysBody(whipAttackHitbox);
            whipAttackHitbox = nullptr;
        }

        // Create new whip attack hitbox
        whipAttackHitbox = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            attackX, centerY, attackWidth, attackHeight, bodyType::DYNAMIC);
        whipAttackHitbox->ctype = ColliderType::PLAYER_ATTACK;
        whipAttackHitbox->listener = this;
    }

    // Existing whip attack state management code remains the same
    if (isWhipAttacking) {
        whipAttack.Update();

        if (whipAttackHitbox) {
            int attackX = facingRight ? position.getX() + 30 : position.getX();
            int attackY = position.getY() + texH / 4;
            whipAttackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }

        if (whipAttack.HasFinished()) {
            isWhipAttacking = false;

            if (whipAttackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(whipAttackHitbox);
                whipAttackHitbox = nullptr;
            }
        }
    }
}

void Player::UpdateMeleeAttack(float dt) {
    // Attack cooldown logic
    if (!canAttack) {
        attackCooldown -= dt;
        if (attackCooldown <= 0.0f) {
            canAttack = true;
            attackCooldown = 0.0f;
        }
    }

    // Initiate melee attack only when not dashing, whip attacking, or already attacking
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN &&
        !isAttacking && !isWhipAttacking && !isDashing && canAttack) {
        // Existing melee attack initialization code remains the same
        isAttacking = true;
        canAttack = false;
        attackCooldown = 0.5f;  // Set cooldown duration

        // Reset and load attack animation
        meleeAttack = Animation();
        meleeAttack.speed = 0.15f;
        meleeAttack.loop = false;
        meleeAttack.LoadAnimations(parameters.child("animations").child("attack"));

        // Create attack hitbox (existing code)
        int attackWidth = texW * 1.1;
        int attackHeight = texH;
        b2Vec2 playerCenter = pbody->body->GetPosition();
        int centerX = METERS_TO_PIXELS(playerCenter.x);
        int centerY = METERS_TO_PIXELS(playerCenter.y);

        // Calculate attack position based on facing direction
        int attackX = facingRight ? centerX + texW / 4 : centerX - texW / 4 - attackWidth / 2;

        // Remove existing attack hitbox
        if (attackHitbox) {
            Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
            attackHitbox = nullptr;
        }

        // Create new attack hitbox
        attackHitbox = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            attackX, attackHeight, attackWidth, attackHeight, bodyType::DYNAMIC);
        attackHitbox->ctype = ColliderType::PLAYER_ATTACK;
        attackHitbox->listener = this;
    }

    // Existing attack state management code remains the same
    if (isAttacking) {
        meleeAttack.Update();

        if (attackHitbox) {
            int attackX = facingRight ? position.getX() + 30 : position.getX();
            int attackY = position.getY() + texH / 2;
            attackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }

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
    // Determine the flip direction based on which way the player is facing
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Prioritize melee attack drawing
    if (isAttacking) {
        // Get the current frame of the attack animation
        SDL_Rect currentFrame = meleeAttack.GetCurrentFrame();
        int attackX;

        // Calculate the position for the attack animation based on facing direction
        if (facingRight) {
            attackX = position.getX();              // Position when facing right
        }
        else {
            attackX = position.getX();              // Offset position when facing left
        }

        // Draw the attack animation with proper positioning and orientation
        Engine::GetInstance().render.get()->DrawTextureWithFlip(
            attackTexture,                          // Attack animation texture
            attackX,                                // X position with direction offset
            position.getY(),                        // Y position with vertical offset
            &currentFrame,                          // Current attack animation frame
            0.0,                                    // Speed modifier (0.0 = no parallax effect)
            0,                                      // Rotation angle (0 = no rotation)
            INT_MAX, INT_MAX,                       // No pivot point (use default)
            flip);                                  // Horizontal flip based on direction
    }
    // Then check for whip attack drawing
    else if (isWhipAttacking) {
        // Get the current frame of the whip attack animation
        SDL_Rect currentFrame = whipAttack.GetCurrentFrame();
        int attackX;

        // Calculate the position for the whip attack animation based on facing direction
        if (facingRight) {
            attackX = position.getX();              // Position when facing right
        }
        else {
            attackX = position.getX();              // Offset position when facing left
        }

        // Draw the whip attack animation with proper positioning and orientation
        Engine::GetInstance().render.get()->DrawTextureWithFlip(
            whipAttackTexture,                      // Whip attack animation texture
            attackX,                                // X position with direction offset
            position.getY(),                        // Y position with vertical offset
            &currentFrame,                          // Current whip attack animation frame
            0.0,                                    // Speed modifier (0.0 = no parallax effect)
            0,                                      // Rotation angle (0 = no rotation)
            INT_MAX, INT_MAX,                       // No pivot point (use default)
            flip);                                  // Horizontal flip based on direction
    }
    // Normal player drawing when not attacking
    else {
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
    if (NeedSceneChange) {//cambio de escena
        Engine::GetInstance().sceneLoader->LoadScene(sceneToLoad, Playerx, Playery,Fade);//pasarle la nueva escena al sceneLoader
        NeedSceneChange = false;
    }

    if (NeedDialogue) {//Dialogo
        Engine::GetInstance().dialogoM->Texto(Id.c_str()); // Llama a Texto que toque
        NeedDialogue = false;
    }

}

bool Player::CleanUp() {
    LOG("Cleanup player");
    Engine::GetInstance().textures.get()->UnLoad(texture);
    Engine::GetInstance().textures.get()->UnLoad(attackTexture);
    Engine::GetInstance().textures.get()->UnLoad(whipAttackTexture);

    return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::ENEMY) {
        LOG("Player attack hit an enemy!");
        // Additional enemy hit logic can go here
        return;
    }
    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        isJumping = false;
        break;
    case ColliderType::ITEM: {
        Item* item = (Item*)physB->listener;
        if (item && item->GetItemType() == "Dash ability") {
            Dash = true;
            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
        if (item && item->GetItemType() == "Whip") {
            WhipAttack = true;
            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
        if (item && item->GetItemType() == "Door key") {
            canOpenDoor = true;
            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
    }

        break;
    case ColliderType::SENSOR:
        LOG("SENSOR COLLISION DETECTED");
        LOG("Sensor ID: %s", physB->sensorID.c_str());
        NeedSceneChange = true;
        for (const auto& escena : escenas) {//recorrer todas las escenas
            if (escena.escena == physB->sensorID) {//mirar donde tiene que ir 
                sceneToLoad = escena.id;
                Playerx = escena.x;
                Playery = escena.y;// Devuelve el ID para cargar ese mapaa
                Fade = escena.fade;
            }
        }
        break;
    case ColliderType::ASCENSORES:
        LOG("ASCENSOR COLLISION DETECTED");
        LOG("Sensor ID: %s", physB->sensorID.c_str());
        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
        {
            NeedSceneChange = true;
            for (const auto& escena : escenas) {//recorrer todas las escenas
                if (escena.escena == physB->sensorID) {//mirar donde tiene que ir 
                    sceneToLoad = escena.id;
                    Playerx = escena.x;
                    Playery = escena.y;// Devuelve el ID para cargar ese mapaa
                    Fade = escena.fade;
                }
            }
        }
        break;
    case ColliderType::DIALOGOS:
        LOG("DIALOGOS COLLISION DETECTED");
        LOG("Sensor ID: %s", physB->ID);
        NeedDialogue = true;
        Id = physB->ID;
        break;

    case ColliderType::UNKNOWN:
        break;
    }
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    // Implementaci�n vac�a pero necesaria
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