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
    godMode = false;
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

    idleTexture = texture;

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    currentAnimation = &idle;

    // Add physics to the player - initialize physics body
    //pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() , (int)position.getY(), texW / 2, bodyType::DYNAMIC);
    pbody->listener = this;
    pbody->ctype = ColliderType::PLAYER;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) {
        pbody->body->SetGravityScale(0);
    }

    // Initialize audio effect
    pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");

    // Setup melee attack - ensure proper initialization
    meleeAttack = Animation();

    // Load attack animations carefully and check for errors
    
    meleeAttack.LoadAnimations(parameters.child("animations").child("attack"));
    attackTexture = Engine::GetInstance().textures.get()->Load(parameters.child("animations").child("attack").attribute("texture").as_string());
    
    jump.LoadAnimations(parameters.child("animations").child("jump"));
    jumpTexture = Engine::GetInstance().textures.get()->Load(parameters.child("animations").child("jump").attribute("texture").as_string());
    isPreparingJump = false;
    isJumping = false;
    jumpFrameThreshold = 3; // This is 4th frame (0-indexed)

    walk.LoadAnimations(parameters.child("animations").child("walk"));
    isWalking = false;

    // Inicializar whipAttack igual que meleeAttack (fuera del bloque condicional)
    whipAttack = Animation();

    whipAttack.LoadAnimations(parameters.child("animations").child("whip"));
    whipAttackTexture = Engine::GetInstance().textures.get()->Load(parameters.child("animations").child("whip").attribute("texture").as_string());

    // Set initial state
    isAttacking = false;
    canAttack = true;
    attackCooldown = 0.0f;
    attackHitbox = nullptr;

    // Initialize whip attack state variables explicitly
    isWhipAttacking = false;
    canWhipAttack = true;
    whipAttackCooldown = 0.0f;
    whipAttackHitbox = nullptr;

    // Para pruebas, habilitar temporalmente el ataque whip
    // Elimina esta línea cuando quieras que el jugador tenga que recoger el item primero
    WhipAttack = true;

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
    if (dialogo == false)//Dejar al player quieto cuando hay dialogos por pantalla
    {
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
        if (NeedSceneChange) {//cambio de escena 
            Engine::GetInstance().sceneLoader->LoadScene(sceneToLoad, Playerx, Playery, Fade);//pasarle la nueva escena al sceneLoader
            NeedSceneChange = false;
        }

        if (NeedDialogue) {//Dialogo 
            Engine::GetInstance().dialogoM->Texto(Id.c_str()); // Llama a Texto que toque 
            NeedDialogue = false;
        }

        // Apply the velocity to the player
        pbody->body->SetLinearVelocity(velocity);

        // Update position from physics body
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
    }

    currentAnimation->Update(); 
    HandleSceneSwitching();
    DrawPlayer();
    return true;
}

void Player::HandleMovement(b2Vec2& velocity) {
   
    // Horizontal movement
    isWalking = false;

    // Horizontal movement
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        velocity.x = -0.2f * 16;
        facingRight = false;
        isWalking = true;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        velocity.x = 0.2f * 16;
        facingRight = true;
        isWalking = true;
    }
    // Toggle god mode when F5 is pressed
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_REPEAT) {
        godMode = !godMode; // This properly toggles between true and false
        // Vertical movement
    }

    // Vertical movement when W is pressed and god mode is active
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
        if (godMode == true) { // Use == for comparison, not =
            velocity.y = -0.2 * 16;
        }
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
    // Start jump animation when space is pressed
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping && !isPreparingJump) {
        isPreparingJump = true;

        // Reset jump animation
        jump.Reset();
        currentAnimation = &jump;
    }

    // Check if we're in the preparation phase
    if (isPreparingJump) {
        // If we've reached the threshold frame, apply the actual jump force
        if (jump.GetCurrentFrameIndex() == jumpFrameThreshold) {
            float jumpForce = 5.0f;
            pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);

            // Now we're officially jumping
            isPreparingJump = false;
            isJumping = true;
        }
    }
}
void Player::HandleSceneSwitching() {
    // Level switching controls
    int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1) {//pasar escena 1
        Engine::GetInstance().sceneLoader->LoadScene(1, 257, 527, true);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2) {//pasar escena 2
        Engine::GetInstance().sceneLoader->LoadScene(2, 100, 520, true);
    }
   /* if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_3) == KEY_DOWN && currentLvl != 2) {//pasar escena 3
        Engine::GetInstance().sceneLoader->LoadScene(sceneToLoad, Playerx, Playery, true);
    }*/
}

// Corregir UpdateWhipAttack() para reiniciar correctamente la animación
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
        // Start whip attack
        isWhipAttacking = true;
        LOG("Whip Attack started");
        canWhipAttack = false;
        whipAttackCooldown = 0.7f;

        // Reset the animation instead of recreating it
        whipAttack.Reset();

        // Create whip attack hitbox
        int attackWidth = texW * 2;
        int attackHeight = texH / 2;
        b2Vec2 playerCenter = pbody->body->GetPosition();
        int centerX = METERS_TO_PIXELS(playerCenter.x);
        int centerY = METERS_TO_PIXELS(playerCenter.y);

        // Calculate attack position based on facing direction
        int attackX = facingRight ? centerX + texW / 4 : centerX - texW / 4 - attackWidth / 2;

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

    // Whip attack state management
    if (isWhipAttacking) {
        // Update animation
        whipAttack.Update();

        // Update hitbox position
        if (whipAttackHitbox) {
            int attackX = facingRight ? position.getX() + 30 : position.getX();
            int attackY = position.getY() + texH / 2;
            whipAttackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }

        // Check if animation finished
        if (whipAttack.HasFinished()) {
            LOG("Whip Attack finished");
            isWhipAttacking = false;
            currentAnimation = &idle;  // Explicitly switch back to idle animation
			texture = idleTexture;

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
        // Start attack
        isAttacking = true;
        LOG("Attack started");
        canAttack = false;
        attackCooldown = 0.5f;  // Set cooldown duration

        // Reset the animation instead of recreating it
        meleeAttack.Reset();

        // Create attack hitbox
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
            attackX, centerY, attackWidth, attackHeight, bodyType::DYNAMIC);
        attackHitbox->ctype = ColliderType::PLAYER_ATTACK;
        attackHitbox->listener = this;
    }

    // Attack state management
    if (isAttacking) {
        // Update animation
        meleeAttack.Update();

        // Update hitbox position
        if (attackHitbox) {
            int attackX = facingRight ? position.getX() + 30 : position.getX() + 30;
            int attackY = position.getY() + texH / 2;
            attackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }

        // Check if animation finished
        if (meleeAttack.HasFinished()) {
            LOG("Attack finished");
            isAttacking = false;

            // Reset back to default animation and texture
            currentAnimation = &idle;
            idle.Reset();  // Explicitly reset the idle animation

            if (attackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
                attackHitbox = nullptr;
            }
        }
    }
}

void Player::DrawPlayer() {
    // Store the original texture so we can properly reset it
    SDL_Texture* originalTexture = texture;

    if (isAttacking) {
        currentAnimation = &meleeAttack;
        // Use attack texture when attacking
        texture = attackTexture;
    }
    else if (isWhipAttacking) {
        currentAnimation = &whipAttack;
        // Use whip texture when whip attacking
        texture = whipAttackTexture;
    }
    else if (isJumping || isPreparingJump) {
        // Set the jump animation when jumping or preparing to jump
        currentAnimation = &jump;
        texture = jumpTexture;
    }
    else if (isWalking) {
        // Set walking animation when moving horizontally
        currentAnimation = &walk;

    }
    else {
        // Only change back to idle if we weren't already in idle
        // This avoids resetting the animation constantly

        if (currentAnimation != &idle) {
            currentAnimation = &idle;
            // Reset texture to the original one loaded in Start()
            texture = originalTexture;
        }
    }

    // Determine the flip direction based on which way the player is facing
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Get the current frame
    SDL_Rect frame = currentAnimation->GetCurrentFrame();

    // Calculate offset for flipping (similar to Boss class)
    int offsetX = 0;
    if (!facingRight) {
        offsetX = (frame.w - texW); // Adjust for sprite width difference when flipped
    }

    // Draw the player with the current animation and appropriate texture
    Engine::GetInstance().render.get()->DrawTexture(
        texture,                    // Current texture based on state
        position.getX() - offsetX,  // X position with offset for flipping
        position.getY(),            // Y position
        &frame,                     // Current animation frame
        1.0f,                       // Scale factor
        0.0,                        // No rotation
        INT_MAX, INT_MAX,           // No pivot
        flip);                      // Flip based on direction
}

bool Player::CleanUp() {
    LOG("Cleanup player");
    Engine::GetInstance().textures.get()->UnLoad(texture);
    Engine::GetInstance().textures.get()->UnLoad(attackTexture);
    Engine::GetInstance().textures.get()->UnLoad(whipAttackTexture);

    return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::TERRESTRE) {
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