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
    bodyJoint = nullptr;
    isWakingUp = true;
    hasWokenUp = false;
}

Player::~Player() {
    // Ensure that collisions are released properly
    if (pbodyUpper != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyUpper);
    }
    if (pbodyLower != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyLower);
    }
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
    idleTexture = texture;  // Default texture

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    currentAnimation = &idle;

    // Physics body initialization - now with two circular collisions
    int radius = texW / 3; // Smaller radius than before
    int centerX = (int)position.getX() + texW / 2;
    int centerY = (int)position.getY() + texH / 2;
    // Place upper body higher up
    pbodyUpper = Engine::GetInstance().physics.get()->CreateCircle(
        centerX,
        centerY - radius,  // upper part
        radius,
        bodyType::DYNAMIC);
    pbodyUpper->listener = this;
    pbodyUpper->ctype = ColliderType::PLAYER;
    pbodyUpper->body->SetFixedRotation(true);
    // Lower body (legs)
    pbodyLower = Engine::GetInstance().physics.get()->CreateCircle(
        centerX,
        centerY + radius,  // lower part
        radius,
        bodyType::DYNAMIC);
    pbodyLower->listener = this;
    pbodyLower->ctype = ColliderType::PLAYER;
    pbodyLower->body->SetFixedRotation(true);
    // Disable gravity
    if (!parameters.attribute("gravity").as_bool()) {
        pbodyUpper->body->SetGravityScale(0.5);
        pbodyLower->body->SetGravityScale(0.5);
    }
    // Verify that bodies exist
    if (!pbodyUpper || !pbodyLower || !pbodyUpper->body || !pbodyLower->body) {
        LOG("Error: One or both Box2D bodies are null");
        return false;
    }
    // Create a weld joint between the two circles
    b2WeldJointDef jointDef;
    jointDef.bodyA = pbodyUpper->body;
    jointDef.bodyB = pbodyLower->body;
    jointDef.collideConnected = false;
    // The point where they touch: right between both
    b2Vec2 worldAnchor = pbodyUpper->body->GetWorldCenter();
    worldAnchor.y += radius; // connection point between both
    jointDef.localAnchorA = pbodyUpper->body->GetLocalPoint(worldAnchor);
    jointDef.localAnchorB = pbodyLower->body->GetLocalPoint(worldAnchor);
    jointDef.referenceAngle = 0;
    // Stiffness configuration
    jointDef.stiffness = 1000.0f;
    jointDef.damping = 0.5f;
    bodyJoint = Engine::GetInstance().physics.get()->world->CreateJoint(&jointDef);
    if (bodyJoint) {
        LOG("Weld joint created successfully");
    }
    else {
        LOG("Failed to create weld joint");
    }
    b2Fixture* fixtureUpper = pbodyUpper->body->GetFixtureList();
    b2Fixture* fixtureLower = pbodyLower->body->GetFixtureList();

    if (fixtureUpper && fixtureLower) {
        // Reducir la fricción para evitar quedarse pegado a las paredes
        fixtureUpper->SetFriction(0.005f);  // Valor bajo para evitar adherencia a paredes
        fixtureLower->SetFriction(0.005f);

        //// Opcional: configurar valores específicos para colisiones laterales
        //fixtureUpper->SetRestitution(0.5f);  // Un poco de rebote
        //fixtureLower->SetRestitution(0.5f);
    }
    // Note on weld joint properties:
// A weld joint fuses two bodies together at a specified point, preventing relative movement.
// - stiffness (1000.0f): Controls how rigid the connection is. Higher values create stiffer joints.
// - damping (0.5f): Reduces oscillation. Higher values make the joint more stable but less responsive.
// - referenceAngle (0): Maintains the initial relative angle between bodies.
// - collideConnected (false): Prevents the connected bodies from colliding with each other.
// This setup creates a character with connected upper and lower body parts that move as one unit.

    // Initialize audio effect
    pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");
    punchFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Weapon_Punch_Hit_D.ogg");
    stepFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Concrete_FS_2.wav");
    diedFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/Game_Died_A.ogg");
    hurtFX= Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/hurt.ogg");
	dashFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/dash.ogg");
	whipFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/whip.ogg");
	fallFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/fall.ogg");
	jumpFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/jump.ogg");
    int lowVolume = 5; // Low volume setting (range: 0 to 128)
    int mediumVolume = 15;
    int highVolume = 80;
    Engine::GetInstance().audio.get()->SetFxVolume(punchFX, lowVolume);
    Engine::GetInstance().audio.get()->SetFxVolume(pickCoinFxId, lowVolume);
    Engine::GetInstance().audio.get()->SetFxVolume(stepFX, 2);
    Engine::GetInstance().audio.get()->SetFxVolume(diedFX, 4);
    Engine::GetInstance().audio.get()->SetFxVolume(hurtFX, 1);
	Engine::GetInstance().audio.get()->SetFxVolume(dashFX, 2);
	Engine::GetInstance().audio.get()->SetFxVolume(whipFX, 10);
	Engine::GetInstance().audio.get()->SetFxVolume(fallFX, 4);
	Engine::GetInstance().audio.get()->SetFxVolume(jumpFX, 10);

    // Attack animation
    meleeAttack = Animation();
    meleeAttack.LoadAnimations(parameters.child("animations").child("attack"));
    // Only load a different texture if one is specified
    auto attackNode = parameters.child("animations").child("attack");
    attackTexture = attackNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(attackNode.attribute("texture").as_string()) : texture;

    // Jump animation
    jump.LoadAnimations(parameters.child("animations").child("jump"));
    auto jumpNode = parameters.child("animations").child("jump");
    jumpTexture = jumpNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(jumpNode.attribute("texture").as_string()) : texture;

    falling.LoadAnimations(parameters.child("animations").child("falling"));
    auto fallingNode = parameters.child("animations").child("falling");
    fallingTexture = fallingNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(fallingNode.attribute("texture").as_string()) : texture;

    recovering.LoadAnimations(parameters.child("animations").child("recoveringJump"));
    auto recoveringNode = parameters.child("animations").child("recoveringJump");
    recoveringTexture = recoveringNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(recoveringNode.attribute("texture").as_string()) : texture;
    
    isPreparingJump = false;
    isJumping = false;
    isFalling = false;
    isRecovering = false;
    recoveringTimer = 0.0f;

    isPreparingJump = false;
    isJumping = false;
    jumpFrameThreshold = 3;

    // Walk animation
    walk.LoadAnimations(parameters.child("animations").child("walk"));
    auto walkNode = parameters.child("animations").child("walk");
    walkTexture = walkNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(walkNode.attribute("texture").as_string()) : texture;
    isWalking = false;

    // Whip attack animation
    whipAttack = Animation();
    whipAttack.LoadAnimations(parameters.child("animations").child("whip"));
    auto whipNode = parameters.child("animations").child("whip");
    whipAttackTexture = whipNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(whipNode.attribute("texture").as_string()) : texture;

    // Dash animation
    dash = Animation();
    dash.LoadAnimations(parameters.child("animations").child("dash"));
    auto dashNode = parameters.child("animations").child("dash");
    dashTexture = dashNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(dashNode.attribute("texture").as_string()) : texture;

    originalGravityScale = parameters.attribute("gravity").as_bool() ? 1.0f : 0.0f;

    // Hurt animation
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    auto hurtNode = parameters.child("animations").child("hurt");
    hurtTexture = hurtNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(hurtNode.attribute("texture").as_string()) : texture;

    // Pickup animation
    pickupAnim.LoadAnimations(parameters.child("animations").child("pickup"));
    auto pickupNode = parameters.child("animations").child("pickup");
    pickupTexture = pickupNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(pickupNode.attribute("texture").as_string()) : texture;
    
    // Death animation
    death.LoadAnimations(parameters.child("animations").child("death"));
    auto deathNode = parameters.child("animations").child("death");
    deathTexture = deathNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(deathNode.attribute("texture").as_string()) : texture;


    // Wakeup animation 
    wakeupAnim.LoadAnimations(parameters.child("animations").child("wakeup"));
    auto wakeupNode = parameters.child("animations").child("wakeup");
    wakeupTexture = wakeupNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(wakeupNode.attribute("texture").as_string()) : texture;

    // Initialize wakeup animation
    isWakingUp = true;
    hasWakeupStarted = false;
    wakeupTimer = 0.0f;

    // Set initial state
    isAttacking = false;
    canAttack = true;
    attackCooldown = 0.0f;
    attackHitbox = nullptr;

    // Explicitly initialize whip attack state variables
    isWhipAttacking = false;
    canWhipAttack = true;
    whipAttackCooldown = 0.0f;
    whipAttackHitbox = nullptr;

    // For testing, temporarily enable whip attack
    WhipAttack = false;
    facingRight = true;

    // Set initial animation to wakeup if we haven't woken up yet
    if (isWakingUp && !hasWokenUp) {
        currentAnimation = &wakeupAnim;
        texture = wakeupTexture;
    }
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

    // Handle wakeup animation first
    if (isWakingUp) {
        HandleWakeup(dt);

        // During wakeup, we freeze player position and don't process other inputs
        pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

        // Update position from physics body
        b2Transform pbodyPos = pbodyUpper->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) + 32 - texH / 2);

        // Draw the player with the wakeup animation
        currentAnimation->Update();
        DrawPlayer();
        return true;
    }
 
    if (Engine::GetInstance().entityManager->dialogo == false)// Keep the player idle when dialogues are on screen
    {
        // Initialize velocity vector para ambos cuerpos
        b2Vec2 velocity = b2Vec2(0, pbodyUpper->body->GetLinearVelocity().y);

        if (!parameters.attribute("gravity").as_bool()) {
            velocity = b2Vec2(0, 0);
        }

        if (lives <= 0 && !isDying) {
            isDying = true;

			isAttacking = isWhipAttacking = isDashing = isJumping = isFalling = isRecovering = false;
            pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));
        }

        if (isDying && !hasDied) {
            HandleDeath(dt);
        }
        if (hasDied) {
            HandleSceneSwitching();
			return true;
        }

        // Mutually exclusive action handling
        if (!isAttacking && !isWhipAttacking && !isDashing && !isPickingUp && !isDying) {
            HandleMovement(velocity);
            HandleJump(dt);
        }

        // Handle dash only when not attacking or jumping
        if (!isAttacking && !isWhipAttacking && !isHurt && !isPickingUp && !isDying) {
            HandleDash(velocity, dt);
        }

		// Handle hurt animation
        HandleHurt(dt);
        if (changeMusicCaronte) {
            Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 0.0f);
            changeMusicCaronte = false;
        }
        // Handle attacks only when not dashing
        if (!isDashing && !isPickingUp && !isDying) {
            UpdateMeleeAttack(dt);
        }
        if (!isDashing && !isPickingUp && !isDying && !isJumping && !isFalling && !isRecovering) {
            UpdateWhipAttack(dt);
 
        }
        // If jumping, preserve the vertical velocity
        if (isJumping) {
            velocity.y = pbodyUpper->body->GetLinearVelocity().y;
           
        }

        if (NeedSceneChange) { // Scene change
            Engine::GetInstance().sceneLoader->LoadScene(sceneToLoad, Playerx, Playery, Fade, BossCam); // Pass the new scene to the sceneLoader
            NeedSceneChange = false;
            TocandoAs = false;
        }

        if (NeedDialogue) { // Dialogue
            Engine::GetInstance().dialogoM->Texto(Id.c_str()); // Call the corresponding dialogue line
            NeedDialogue = false;
        }

        Ascensor();

        // Apply the velocity to both bodies
        pbodyUpper->body->SetLinearVelocity(velocity);
        pbodyLower->body->SetLinearVelocity(velocity);

        // Update position from physics body (usamos el cuerpo superior como referencia principal)
        b2Transform pbodyPos = pbodyUpper->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) + 32 - texH / 2);
    }
    else { isWalking = false; currentAnimation = &idle; pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f)); pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));/*stop body*/ }

    // Handle pickup animation
    HandlePickup(dt);

    // Handle animations depending on state
    if (isJumping && !isPreparingJump) {
        // Check vertical velocity to determine if falling
        float verticalVelocity = pbodyUpper->body->GetLinearVelocity().y;

        // If moving downward (positive y velocity), switch to falling animation
        if (verticalVelocity > 0.1f && !isFalling) {
            LOG("Transition to falling animation");
            isFalling = true;
            falling.Reset();
        }
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_P) == KEY_REPEAT) {
        godMode = !godMode; // This properly toggles between true and false
        // Vertical movement
    }
    currentAnimation->Update();
    DrawPlayer();

    HandleSceneSwitching();

    return true;
}

bool Player::PostUpdate() {
    return true; 
}

void Player::HandleWakeup(float dt) {
    if (!hasWakeupStarted) {
        wakeupAnim.Reset();
        hasWakeupStarted = true;
        texture = wakeupTexture;
        currentAnimation = &wakeupAnim;

        // Disable player movement during wakeup
        pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

        LOG("Starting wakeup animation");
    }

    wakeupAnim.Update();

    // Once the animation completes, return to idle state
    if (wakeupAnim.HasFinished()) {
        isWakingUp = false;
        hasWakeupStarted = false;
        hasWokenUp = true;
        currentAnimation = &idle;
        texture = idleTexture;
        LOG("Wakeup animation completed");
    }
}

void Player::HandleMovement(b2Vec2& velocity) {

    float verticalVelocity = velocity.y;

    // Horizontal movement
    isWalking = false;
    velocity.x = 0; // Reset horizontal velocity

    // Apply horizontal movement
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        velocity.x = -0.3f * 16;
        facingRight = false;
        isWalking = true;
    }

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        velocity.x = 0.3f * 16;
        facingRight = true;
        isWalking = true;
    }

    // Restore vertical velocity after horizontal input
    if (isJumping || isFalling) {
        velocity.y = verticalVelocity;
    }

    // Vertical movement when W is pressed and god mode is active
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
        if (godMode == true) { 
            velocity.y = -0.3 * 16;
        }
    }
}

void Player::HandleDash(b2Vec2& velocity, float dt) {
    // Update dash cooldown if it's active
    if (!canDash) {
        dashCooldown -= dt;
        if (dashCooldown <= 0.0f) {
            canDash = true;
            Engine::GetInstance().audio.get()->PlayFx(dashFX);
            dashCooldown = 0.0f;
        }
    }

    bool isOnGround = !isJumping;

    
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN &&
        canDash && Dash && !isAttacking && !isWhipAttacking && !isDashing) {

        // Check if we still have dashes available
        if (totalDashesAvailable > 0) {
            
            totalDashesAvailable--; // Reduce the number of available dashes

            dash.Reset();
            currentAnimation = &dash;
            texture = dashTexture;
            isDashing = true;
            canDash = false;
            dashCooldown = 1.5f;
            dashFrameCount = 20;
            dashSpeed = 12.0f;
            dashDirection = facingRight ? 1.0f : -1.0f;
        }
    }

    // dash recharge
    if (dashRechargeTimer > 0) {
        dashRechargeTimer -= dt;
        if (dashRechargeTimer <= 0.0f) {
            // Full recharge
            totalDashesAvailable = maxTotalDashes;
            dashRechargeTimer = 0.0f;
        }
    }

    
    if (totalDashesAvailable < maxTotalDashes && dashRechargeTimer <= 0.0f) {
        dashRechargeTimer = dashFullRechargeTime;
    }

   
    if (isDashing) {
        velocity.x = dashDirection * dashSpeed;
        dashFrameCount--;
        if (dashFrameCount <= 0) {
            isDashing = false;
        }
    }
}

void Player::HandleJump(float dt) {
    float currentVerticalVelocity = pbodyUpper->body->GetLinearVelocity().y;
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN &&
        !isJumping && !isFalling) {

        // Apply jump force immediately
        float jumpForce = 3.5f;

        // Apply impulse to both bodies
        pbodyUpper->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
        pbodyLower->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);

        // Set jump state
        isJumping = true;
        isFalling = false;
        Engine::GetInstance().audio.get()->PlayFx(jumpFX);

        // Reset and start jump animation
        jump.Reset();
        currentAnimation = &jump;
        texture = jumpTexture;
    }

    // Check vertical velocity to determine if falling
    if (isJumping) {
        float verticalVelocity = pbodyUpper->body->GetLinearVelocity().y;

        if (verticalVelocity > 0.005f && !isFalling) {
            isFalling = true;
            isTransitioningToFalling = true;  // Mark that we're in transition
            falling.Reset();
            
        }
    }
    if (isFalling) {
        // Check both bodies' vertical velocity to determine if player has actually stopped falling
        float upperVelocity = pbodyUpper->body->GetLinearVelocity().y;
        float lowerVelocity = pbodyLower->body->GetLinearVelocity().y;

        // If both bodies have very little vertical movement and the player isn't intentionally jumping
        if (fabs(upperVelocity) < 0.1f && fabs(lowerVelocity) < 0.1f && !isJumping) {
            // We've probably landed but the collision wasn't registered properly
            isFalling = false;
            isJumping = false;
            falling.Reset();

            // Transition to recovering animation
            isRecovering = true;
            recoveringTimer = recoveringDuration;
            recovering.Reset();
        }

        fallingTimer += dt;
        if (fallingTimer > 100.0f) {  // If we've been falling for over x seconds
            // Check if we're not actually moving much
            if (fabs(upperVelocity) < 0.5f && fabs(lowerVelocity) < 0.5f) {
                isFalling = false;
                isJumping = false;
                fallingTimer = 0.0f;
            }
        }
    }
    else {
        // Reset falling timer when not falling
        fallingTimer = 0.0f;
    }
    // Check if player is falling off a platform without jumping
    if (!isJumping && !isDashing && !isAttacking && !isWhipAttacking && !isRecovering) {
        float verticalVelocity = pbodyUpper->body->GetLinearVelocity().y;
        if (verticalVelocity > 1.0f) { // More significant threshold for non-jump falling
            isJumping = true; // Technically in "air" state
            isFalling = true;
            falling.Reset();
        }
    }
}

void Player::HandleSceneSwitching() {
    // Level switching controls
    int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1 || hasDied && currentLvl == 1) {//go to scene 1
        Engine::GetInstance().sceneLoader->LoadScene(1, 2900, 2079, false, false);
		hasDied = false;    
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2 || hasDied && currentLvl == 2) {//go to scene 2
        Engine::GetInstance().sceneLoader->LoadScene(2, 3082, 720, false, false);
        hasDied = false;
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_3) == KEY_DOWN && currentLvl != 3 || hasDied && currentLvl == 3) {//go to scene 3
        Engine::GetInstance().sceneLoader->LoadScene(3, 700, 600, false, false);
        hasDied = false;
    }
    // unlocks sensors
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_0) == KEY_DOWN) {//unlocks sensors scene change
        DesbloquearSensor();
        Engine::GetInstance().entityManager->AscensorOn();
    }
    //Debug Level Design
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN) { //Move the player to the start position tunel scene
        int debugXLevelDesign = 2893;
        int debugYLevelDesign = 2048;
        Vector2D debugPos(debugXLevelDesign, debugYLevelDesign);
        SetPosition(debugPos);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F8) == KEY_DOWN) { //Move the player yo de dash item
        int debugXLevelDesignInitialPositionOfLevel = 6600;
        int debugYLevelDesignInitialPositionOfLevel = 4426;
        Vector2D debugPos(debugXLevelDesignInitialPositionOfLevel, debugYLevelDesignInitialPositionOfLevel);
        SetPosition(debugPos);
    }
}
void Player::HandleHurt(float dt) {
    // Check if the player is hurt
    if (isHurt && !isDying) {
        if (!hasHurtStarted) {
            hurt.Reset();
            hasHurtStarted = true; // Reset hurt animation one time
            Engine::GetInstance().audio.get()->PlayFx(hurtFX);

            // Reset attack states when hurt
            isAttacking = false;
            if (attackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
                attackHitbox = nullptr;
            }

            // Reset whip attack states when hurt
            isWhipAttacking = false;
            if (whipAttackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(whipAttackHitbox);
                whipAttackHitbox = nullptr;
            }

            // Ensure we can attack again after being hurt
            canAttack = true;
            attackCooldown = 0.0f;
            canWhipAttack = true;
            whipAttackCooldown = 0.0f;
        }

        hurt.Update();
        if (hurt.HasFinished()) {
            // Reset to idle
            hurted = true;
            isHurt = false;
            hasHurtStarted = false;
            currentAnimation = &idle;
            idle.Reset();
        }
    }
}

void Player::HandlePickup(float dt) {
    if (isPickingUp && !isDying) {
        LOG("pillando");
        if (!hasPickupStarted) {
            LOG("Empezo a pillar");
            pickupAnim.Reset();
            hasPickupStarted = true; // Reset pickup animation one time
        }
        pickupAnim.Update(); 
        LOG("Acabo de pillar");

        if (pickupAnim.HasFinished()) { 
            LOG("Reseteo pos pillar");
            // Reset to idle
            isPickingUp = false;
            hasPickupStarted = false;
            idle.Reset();
            currentAnimation = &idle;
            
        }
    }
}

void Player::HandleDeath(float dt) {
    if (!hasDeathStarted) {
        death.Reset();
        hasDeathStarted = true;
		currentAnimation = &death;
		texture = deathTexture;
    }
    death.Update();

    if (death.HasFinished()) {
		hasDied = true;
        isDying = false;
		hasDeathStarted = false;
        lives = 3;
		isWakingUp = true;
		currentAnimation = &wakeupAnim;
		wakeupAnim.Reset();
    }
}

// Correct UpdateWhipAttack() to restart the animation correctly
void Player::UpdateWhipAttack(float dt) {

    if (isHurt || isDying) {
        if (isWhipAttacking) {
            isWhipAttacking = false;
            if (whipAttackHitbox) {
                
                Engine::GetInstance().physics.get()->DeletePhysBody(whipAttackHitbox);
                whipAttackHitbox = nullptr;
            }
           
            currentAnimation = &idle;
            idle.Reset();
        }
        return;
    }

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
        !isWhipAttacking && !isAttacking && !isDashing && canWhipAttack && WhipAttack && !isHurt) {
        // Start whip attack
        Engine::GetInstance().audio.get()->PlayFx(whipFX);
        isWhipAttacking = true;
        canWhipAttack = false;
        whipAttackCooldown = 0.7f;

        // Reset the animation instead of recreating it
        whipAttack.Reset();

        // Create whip attack hitbox
        int attackWidth = texW * 3;
        int attackHeight = texH / 2;
        b2Vec2 playerCenter = pbodyUpper->body->GetPosition();  // Use upper body for attacks
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
        whipAttackHitbox->ctype = ColliderType::PLAYER_WHIP_ATTACK;
        whipAttackHitbox->listener = this;
    }

    // Whip attack state management
    if (isWhipAttacking) {
        // Update animation
        whipAttack.Update();

        // Update hitbox position
        if (whipAttackHitbox) {
            //int attackX = facingRight ? position.getX()  : position.getX() + 10;
            int attackX = (int)position.getX() + (facingRight ? 150 : -50);
            int attackY = position.getY() + 10;
            whipAttackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }

        // Check if animation finished
        if (whipAttack.HasFinished()) {
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

    if (isHurt || isDying) {
        if (isAttacking) {
            isAttacking = false;
            if (attackHitbox) {
                Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
                attackHitbox = nullptr;
            }
            currentAnimation = &idle;
            idle.Reset();
        }
        return;
    }

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
        !isAttacking && !isWhipAttacking && !isDashing && canAttack && !isHurt) {
        Engine::GetInstance().audio.get()->PlayFx(punchFX);
        isAttacking = true;
        canAttack = false;
        attackCooldown = 0.5f;
        meleeAttack.Reset();

        // Calculate the size of the hitbox to fit the attack sprite.
        int attackWidth = texW * 0.5f;
        int attackHeight = texH * 1.0f;

        b2Vec2 playerCenter = pbodyUpper->body->GetPosition();  // Use upper body
        int centerX = METERS_TO_PIXELS(playerCenter.x);
        int centerY = METERS_TO_PIXELS(playerCenter.y);

        // Hitbox position: in front of the player according to the direction.
        int attackX = facingRight
            ? centerX + texW / 2 - attackWidth / 2 // right
            : centerX - texW / 2 - attackWidth / -2;  // left

        // Position the hitbox lower by using a positive value for texH division
        int attackY = centerY + texH / 4;  // Move hitbox down by half texture height

        // Delete the previous hitbox if it existed.
        if (attackHitbox) {
            Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
            attackHitbox = nullptr;
        }

        // Create a new hitbox and store dimensions for debugging.
        attackHitbox = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            attackX, attackY, attackWidth, attackHeight - 64, bodyType::DYNAMIC);
        attackHitbox->ctype = ColliderType::PLAYER_ATTACK;
        attackHitbox->listener = this;
        attackHitbox->width = attackWidth;
        attackHitbox->height = attackHeight;
    }

    // Attack management
    if (isAttacking) {
        meleeAttack.Update();

        if (attackHitbox) {
            int centerX = METERS_TO_PIXELS(pbodyUpper->body->GetPosition().x);
            int centerY = METERS_TO_PIXELS(pbodyUpper->body->GetPosition().y);

            // Hitbox position adjusted with the direction corrected.
            int attackX = facingRight
                ? centerX + texW / 2 - attackHitbox->width / -3 // right
                : centerX - texW / 2 - attackHitbox->width / -2; // left

            // Keep the hitbox consistently positioned lower
            int attackY = centerY + texH / 4;  // Same offset as in creation

            attackHitbox->body->SetTransform(
                { PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }

        if (meleeAttack.HasFinished()) {
            isAttacking = false;
            currentAnimation = &idle;
            idle.Reset();

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
    if (isDying) {
        texture = deathTexture;
        currentAnimation = &death;
        Engine::GetInstance().audio.get()->PlayFx(diedFX);
        death.Update();
    }
    else if (isHurt && !hurted && !isDying) {
        texture = hurtTexture;
        currentAnimation = &hurt;
        hurt.Update();
    }
    else if (isWakingUp) {
        currentAnimation = &wakeupAnim;
        texture = wakeupTexture;
        wakeupAnim.Update();
    }
    else if (isAttacking) {
        currentAnimation = &meleeAttack;
        texture = attackTexture;
        meleeAttack.Update();
    }
    else if (isWhipAttacking) {
        currentAnimation = &whipAttack;
        texture = whipAttackTexture;
        whipAttack.Update();
    }
    else if (isDashing) {
        currentAnimation = &dash;
        texture = dashTexture;
        dash.Update();
    }
    else if (isPickingUp) {
        currentAnimation = &pickupAnim;
        texture = pickupTexture;
        pickupAnim.Update();
    }
    else if (isJumping || isFalling) {
        // Make sure we never switch back to idle during transitions
        if (isTransitioningToFalling || isFalling) {
            if (currentAnimation != &falling) {
                currentAnimation = &falling;
                texture = fallingTexture;
            }
            falling.Update();

            // Once we've made the transition, clear the flag
            if (isTransitioningToFalling) {
                isTransitioningToFalling = false;
            }
        }
        else {
            if (currentAnimation != &jump) {
                currentAnimation = &jump;
                texture = jumpTexture;
            }
            jump.Update();
        }
    }

    else if (isRecovering) {
        currentAnimation = &recovering;
        texture = recoveringTexture;
        recovering.Update();

        recoveringTimer -= 0.0167f;
        if (recoveringTimer <= 0 || recovering.HasFinished()) {
            isRecovering = false;
            currentAnimation = &idle;
            idle.Reset();
        }
    }
    else if (isWalking) {
        currentAnimation = &walk;
        texture = walkTexture;
        if (!isJumping && isWalking && !isDashing) {
            runSoundTimer += 0.0167;
            if (runSoundTimer >= runSoundInterval) {
                Engine::GetInstance().audio.get()->PlayFx(stepFX);
                runSoundTimer = 0.0f;
            }
        }
        walk.Update();
    }
    else {
        if (currentAnimation != &idle) {
            currentAnimation = &idle;
            texture = originalTexture;
        }
    }
    // Determine the flip direction based on which way the player is facing
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Get the current frame
    SDL_Rect frame = currentAnimation->GetCurrentFrame();
    int offsetX = 15;
    int yOffset = 15;
    int drawX = position.getX() + offsetX;
    int drawY = position.getY() - yOffset;
    // Calculate offset for flipping (similar to Boss class)
    if (isWhipAttacking) {

		if (facingRight) {
		
        drawX = position.getX() - 32;
        drawY = position.getY() - 79;
       
		}
		else {

            drawX = position.getX() - 128;
            drawY = position.getY() - 79;
           
        }
    }
    if (isJumping || isFalling || isRecovering ) {
        if (facingRight) {

            drawX = position.getX() - 7 ;
            drawY = position.getY() - 10;
        }
        else {

            drawX = position.getX() +  7;
            drawY = position.getY() - 10 ;

        }
    }

    if (!facingRight) {
        offsetX = (frame.w - texW); // Adjust for sprite width difference when flipped
    }

    // Draw the player with the current animation and appropriate texture
    Engine::GetInstance().render.get()->DrawTexture(
        texture,                    // Current texture based on state
        drawX,                      // X position with offset for flipping
        drawY,                      // Y position
        &frame,                     // Current animation frame
        1.0f,                       // Scale factor
        0.0,                        // No rotation
        INT_MAX, INT_MAX,           // No pivot
        flip);                      // Flip based on direction

    // Debug drawing of collision bodies (descomentar para debugging)
    /*
    SDL_Color upperColor = { 255, 0, 0, 255 };  // Rojo para cuerpo superior
    SDL_Color lowerColor = { 0, 0, 255, 255 };  // Azul para cuerpo inferior

    int upperRadius = texW / 3;
    int lowerRadius = texW / 3;

    // Dibuja círculos de colisión
    Engine::GetInstance().render.get()->DrawCircle(
        METERS_TO_PIXELS(pbodyUpper->body->GetPosition().x),
        METERS_TO_PIXELS(pbodyUpper->body->GetPosition().y),
        upperRadius, upperColor);

    Engine::GetInstance().render.get()->DrawCircle(
        METERS_TO_PIXELS(pbodyLower->body->GetPosition().x),
        METERS_TO_PIXELS(pbodyLower->body->GetPosition().y),
        lowerRadius, lowerColor);
    */
}

bool Player::CleanUp() {
    LOG("Cleanup player");

    // Cleanup physics bodies
    if (pbodyUpper != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyUpper);
        pbodyUpper = nullptr;
    }

    if (pbodyLower != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyLower);
        pbodyLower = nullptr;
    }
    // Unload textures
    Engine::GetInstance().textures.get()->UnLoad(texture);
    Engine::GetInstance().textures.get()->UnLoad(attackTexture);
    Engine::GetInstance().textures.get()->UnLoad(whipAttackTexture);

    return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (physA->ctype == ColliderType::PLAYER_ATTACK && physB->ctype == ColliderType::TERRESTRE) {
        // Additional enemy hit logic can go here
        return;
    }
    if (physA->ctype == ColliderType::PLAYER_WHIP_ATTACK && physB->ctype == ColliderType::LEVER) {
        Levers* lever = (Levers*)physB->listener;
        if (lever && lever->GetLeverType() == "lever" && !leverOne) {
            leverOne = true;
            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
        if (lever && lever->GetLeverType() == "lever_memory_left" && !leverTwo) {
            leverTwo = true;
            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
        if (lever && lever->GetLeverType() == "lever_to_Station_E1L1" && !leverThree) {
                leverThree = true;
                Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
        if (lever && lever->GetLeverType() == "lever_next_to_dash" && !leverFour) {
            leverFour = true;
            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        }
        return;
    }

    if (physA->ctype == ColliderType::PLAYER  &&  physB->ctype == ColliderType::ITEM) {
        Item* item = (Item*)physB->listener;

        if (item) {

            isPickingUp = true;

            if (item && item->GetItemType() == "Dash ability") {
                Dash = true;
                Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);

            }
            if (item && item->GetItemType() == "Whip") {
                WhipAttack = true;
                Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);

            }
            if (item && item->GetItemType() == "Remember1") {
                Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
                NeedDialogue = true; //activate dialog when touching item, in the xml put the id of the dialog to be activated
                Id = physB->ID; //ID from Item
            }
            if (item && item->GetItemType() == "Door key") {
                canOpenDoor = true;
                Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
                changeMusicCaronte = true; 
                //Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 1.0f);

            }
        }
        return;
    }

    if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::DIALOGOS) {
        if (physB->Salio == false) {//makes the dialog trigger only once
            NeedDialogue = true;
            Id = physB->ID;
            physB->Salio = true;

        }
        return;
    }

    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        // Get the contact point and normal to better determine if we landed on top
        if (isJumping || isFalling) {
            // Check vertical velocity to confirm we're moving downward (landing)
            float verticalVelocity = pbodyUpper->body->GetLinearVelocity().y;

            // Only consider it a landing if we're moving downward (positive Y velocity)
            if (verticalVelocity >= 0) {
                // Force reset both states at the same time
                isJumping = false;
                isFalling = false;
                fallingTimer = 0.0f;  // Reset the timer

                // Transition directly to recovering animation
                isRecovering = true;
                recoveringTimer = recoveringDuration;
                recovering.Reset();

                // Play the landing sound
                Engine::GetInstance().audio.get()->PlayFx(fallFX);
            }
        }
        break;
    case ColliderType::ITEM: {
        //Item* item = (Item*)physB->listener;

        //if (item) {

        //    isPickingUp = true;

        //    if (item && item->GetItemType() == "Dash ability") {
        //        Dash = true;
        //        Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        //        /* NeedDialogue = true; //activate dialog when touching item, in the xml put the id of the dialog to be activated
        //        Id = physB->ID;*/ //ID from Item
        //        break;
        //    }
        //    if (item && item->GetItemType() == "Whip") {
        //        WhipAttack = true;
        //        Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        //        NeedDialogue = true; //activate dialog when touching item, in the xml put the id of the dialog to be activated
        //        Id = physB->ID; //ID from Item
        //        break;
        //    }
        //    if (item && item->GetItemType() == "Door key") {
        //        canOpenDoor = true;
        //        Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
        //        Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 1.0f);
        //        /* NeedDialogue = true; //activate dialog when touching item, in the xml put the id of the dialog to be activated
        //        Id = physB->ID;*/ //ID from Item
        //        break;
        //    }
        //}
        break;
    }
    case ColliderType::BOSS_ATTACK: {
        if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
            isHurt = true;
            lives--;
			// Cancel any ongoing attack
            if (isAttacking) {
                isAttacking = false;
                if (attackHitbox) {
                    Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
                    attackHitbox = nullptr;
                }
            }
            if (isWhipAttacking) {
                isWhipAttacking = false;
                if (whipAttackHitbox) {
                    Engine::GetInstance().physics.get()->DeletePhysBody(whipAttackHitbox);
                    whipAttackHitbox = nullptr;
                }
            }
        }

        break;
    }
    case ColliderType::LEVER: {
        break;
    }
    case ColliderType::SENSOR:
        NeedSceneChange = true;
        for (const auto& escena : escenas) { // Iterate through all scenes
            if (escena.escena == physB->sensorID) { // Check where the player needs to go
                sceneToLoad = escena.id;
                Playerx = escena.x;
                Playery = escena.y; // Set the destination map and player position
                Fade = escena.fade;
                BossCam = escena.CamaraBoss;
            }
        }
        break;
    case ColliderType::ASCENSORES:
        if (Bloqueo == false) {
            TocandoAs = true;
            for (const auto& escena : escenas) { // Iterate through all scenes
                if (escena.escena == physB->sensorID) { // Check where the player needs to go
                    sceneToLoad = escena.id;
                    Playerx = escena.x;
                    Playery = escena.y; // Set the ID and coordinates to load that map
                    Fade = escena.fade;
                    BossCam = escena.CamaraBoss;
                }
            }
        }
        break;
    case ColliderType::DIALOGOS:
        //if (physB->Salio == false) {//makes the dialog trigger only once
        //    NeedDialogue = true;
        //    Id = physB->ID;
        //    physB->Salio = true;

        //}
        break;
    case ColliderType::UNKNOWN:
        break;
    }
}

void Player::Ascensor() {
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN && TocandoAs == true)
    {
        NeedSceneChange = true;
    }
}

void Player::BloquearSensor(){//block scene change sensors to prevent the player from escaping
    Bloqueo = true;
}

void Player::DesbloquearSensor(){//unlocks sensors scene change
    Bloqueo = false;
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    // We only process collisions of the player's bodies.
    if (physA != pbodyUpper && physA != pbodyLower) {
        return;
    }

    switch (physB->ctype) {
    case ColliderType::PLATFORM:
        // If we're not in a jump or falling state, we might be starting to fall
        if (!isJumping && !isFalling && !isPreparingJump && !isDashing) {
            // Check if we're moving away from a platform
            float verticalVelocity = pbodyUpper->body->GetLinearVelocity().y;
            if (verticalVelocity > 0.1f) {
                isJumping = true;  // We're in air
                isFalling = true;
                falling.Reset();
            }
        }
        break;
    case ColliderType::BOSS_ATTACK:
        if (isHurt) {
            isHurt = false;
            hasHurtStarted = false;
            hurted = false;
        }
        break;
    case ColliderType::ASCENSORES:
        TocandoAs = false;
        break;
    case ColliderType::DIALOGOS:
        physB->Salio = true;
        break;
    }
}

void Player::SetPosition(Vector2D pos) {
    // Establish upper body position
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 3);  // Locate at one third of the total height.
    b2Vec2 upperPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbodyUpper->body->SetTransform(upperPos, 0);

    // Establish lower body position
    Vector2D lowerPos = pos;
    lowerPos.setY(pos.getY() + texH / 3);  // One third lower than the upper
    b2Vec2 lowerPosB2 = b2Vec2(PIXEL_TO_METERS(lowerPos.getX()), PIXEL_TO_METERS(lowerPos.getY()));
    pbodyLower->body->SetTransform(lowerPosB2, 0);
}

Vector2D Player::GetPosition() {
    // Use upper body position as a reference
    b2Vec2 bodyPos = pbodyUpper->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}