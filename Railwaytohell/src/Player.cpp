
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
#include "Explosivo.h"
#include "UI.h"
#include "Mapa.h"
#include "GlobalSettings.h"

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

    SaveInitialPosition();

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
    }
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
    itemFX = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/itemfx.ogg");
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
    Engine::GetInstance().audio.get()->SetFxVolume(itemFX, 10);

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
    
    isFalling = false;
    isRecovering = false;
    recoveringTimer = 0.0f;

    isPreparingJump = false;
    isJumping = false;
    jumpFrameThreshold = 3;
    jumpCount = 0;
    canDoubleJump = true;

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

    // Throw animation
    throwAnim.LoadAnimations(parameters.child("animations").child("throw"));
    auto throwNode = parameters.child("animations").child("throw");
    throwTexture = throwNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(throwNode.attribute("texture").as_string()) : texture;

    // Slide animation
    slide.LoadAnimations(parameters.child("animations").child("slide"));
    auto slideNode = parameters.child("animations").child("slide");
    slideTexture = slideNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(slideNode.attribute("texture").as_string()) : texture;

    // Fall strong animation
    strongfall.LoadAnimations(parameters.child("animations").child("strongfall"));
    auto strongfallNode = parameters.child("animations").child("strongfall");
    strongfallTexture = strongfallNode.attribute("texture") ? Engine::GetInstance().textures.get()->Load(strongfallNode.attribute("texture").as_string()) : texture;

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
	//Dash = true;
    //facingRight = true;
 
    //Ice Movement
    giro = facingRight;

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

    if (Engine::GetInstance().scene->IsPaused()) {
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
    if (freezeWhileHurting) {
   
        HandleHurt(dt);

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
            //UI Lives
            Engine::GetInstance().ui->figth = false;
            Engine::GetInstance().ui->figth2 = false;
            Engine::GetInstance().ui->figth3 = false;
            Engine::GetInstance().scene->SalirBoss();
            Engine::GetInstance().scene->DesbloquearSensor();
            HandleSceneSwitching();
            hasDied = false;
			return true;
        }

        // Mutually exclusive action handling
        if (!isAttacking && !isWhipAttacking && !isDashing && !isPickingUp && !isDying && !isRecovering) {
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
        if (!isDashing && !isPickingUp && !isDying && !collidingWithEnemy) {
            HandleBallAttack(dt);
        }
        HitWcooldown(dt);
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
        if (isHurt && waitForHurtAnimation) {
            if (hurt.HasFinished()) {
                isHurt = false;
                hasHurtStarted = false;
                hurted = true; // Mark that we've been hurt
            }
        }
        Abyss();
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

        // If moving downward (positive and velocity), switch to falling animation
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
    //UI Lives
    Engine::GetInstance().ui->vidap = lives;
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
    isWalking = false;
    // Horizontal movement
    if (resbalar == true) {//Ice platform
            if (facingRight == true) {
                velocity.x = icev; //horizontal velocity on ice rigth
            }
            else {
                velocity.x = -icev; //horizontal velocity on ice left
            }

    }else{
        velocity.x = 0; //horizontal velocity
    }

    auto& engine = Engine::GetInstance();
    auto input = engine.input.get();

    // Check controller input
    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();

        // Left analog stick horizontal axis (for movement)
        Sint16 axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);

        // Define a dead zone for the analog stick
        const int JOYSTICK_DEAD_ZONE = 8000;

        if (axisX < -JOYSTICK_DEAD_ZONE) {
            // Movement proportional to stick tilt
            float axisValue = abs(axisX) / 32768.0f; // Normalized between 0 and 1
            velocity.x = -0.3f * 16 * axisValue;
            facingRight = false;
            isWalking = true;
        }
        else if (axisX > JOYSTICK_DEAD_ZONE) {
            float axisValue = axisX / 32768.0f; // Normalized between 0 and 1
            velocity.x = 0.3f * 16 * axisValue;
            facingRight = true;
            isWalking = true;
        }

        // D-pad buttons
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            b = 0;
            antb = 2;
            if (resbalar == true) {//Ice platform
                if (velocity.x < 0.0f && a < dificultty && giro == true) {
                    if (primero == true) {
                        velocity.x = 4.0;
                        if (a == 10) { primero = false; }

                    }
                    else {
                        velocity.x = velocity.x - anta;
                        anta = anta + 1;
                    }
                    a++;
                }else {
                    velocity.x = -0.3f * 16 - icev;//adding vel to speed up on ice
                    giro = facingRight;
                }
            }else{
                velocity.x = -0.3f * 16;
                giro = facingRight;
            }
            facingRight = false;
            isWalking = true;
        }
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            a = 0;
            anta = 2;
            primero = true;
            if (resbalar == true) {//Ice platform
                if (velocity.x > 0.0f && b < dificultty && giro == false) {
                    if (primerob) {
                        velocity.x = -4.0;
                        if (b == 10) { primerob = false; }

                    }
                    else {
                        velocity.x = velocity.x + antb;
                        antb = antb + 1;
                    }
                    b++;
                }else{
                    velocity.x = 0.3f * 16 + icev;//adding vel to speed up on ice
                    giro = facingRight;
                }
            }else{
                velocity.x = 0.3f * 16;
                giro = facingRight;
            }
            facingRight = true;
            isWalking = true;
        }
    }

    // Check keyboard input (maintain compatibility)
    if (input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
        b = 0;
        antb = 0.5;
        primerob = true;
        if (resbalar == true) {//Ice platform
            if (velocity.x < 0.0f && a < dificultty && giro == true) {             
                if (primero == true) {
                    velocity.x = 4.0;
                    if(a == 10){ primero = false; }
                   
                }
                else {
                    velocity.x = velocity.x - anta;
                    anta = anta + 1;
                }
                a++;
            }else{
                velocity.x = -0.3f * 16 - icev;//adding vel to speed up on ice
                giro = facingRight;
            }
        }else{
            velocity.x = -0.3f * 16;
            giro = facingRight;
        }
        facingRight = false;
        isWalking = true;
    }
    if (input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
        a = 0;
        anta = 0.5;
        primero = true;
        if (resbalar == true) {//Ice platform
            if (velocity.x > 0.0f && b < dificultty && giro == false) {
                if (primerob) {
                    velocity.x = -4.0;
                    if(b == 10){ primerob = false; }
                    
                }
                else {
                    velocity.x = velocity.x + antb;
                    antb = antb + 1;
                }
                b++;
            }else{
                velocity.x = 0.3f * 16 + icev;//adding vel to speed up on ice
                giro = facingRight;
            }
        }else{
            velocity.x = 0.3f * 16;
            giro = facingRight;
        }
        facingRight = true;
        isWalking = true;
    }

    // Restore vertical velocity after horizontal input
    if (isJumping || isFalling) {
        velocity.y = verticalVelocity;
    }

    // Vertical movement when W is held or up is pressed on the controller and godMode is active
    bool upPressed = false;

    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();
        // Check D-pad up button
        upPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);

        // Check analog stick (if not triggered by D-pad)
        if (!upPressed) {
            Sint16 axisY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            const int JOYSTICK_DEAD_ZONE = 8000;
            upPressed = (axisY < -JOYSTICK_DEAD_ZONE);
        }
    }

    if ((input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT || upPressed) && godMode) {
        velocity.y = -0.3f * 16;
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
    auto& engine = Engine::GetInstance();
    bool dashButtonPressed = false;

    // Check controller input for dash (R1 button)
    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();
        // R1 is SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
            dashButtonPressed = true;
        }
    }

    // Maintain keyboard compatibility (LSHIFT key)
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN) {
        dashButtonPressed = true;
    }

    // Dash activation
    if (dashButtonPressed && canDash && Dash && !isAttacking && !isWhipAttacking && !isDashing) {
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

    // Dash recharge logic
    if (dashRechargeTimer > 0) {
        dashRechargeTimer -= dt;
        if (dashRechargeTimer <= 0.0f) {
            // Full recharge
            totalDashesAvailable = maxTotalDashes;
            dashRechargeTimer = 0.0f;
        }
    }

    // Start recharge if dashes are available
    if (totalDashesAvailable < maxTotalDashes && dashRechargeTimer <= 0.0f) {
        dashRechargeTimer = dashFullRechargeTime;
    }

    // Handle dash movement
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
    auto& engine = Engine::GetInstance();
    bool jumpButtonPressed = false;

    // Check controller input for jump (X button)
    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();
        bool AbuttonPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        // Store previous button state to detect when it is newly pressed
        static bool previousAbuttonPressed = false;
        if (AbuttonPressed && !previousAbuttonPressed) {
            jumpButtonPressed = true;
        }
        previousAbuttonPressed = AbuttonPressed;
    }

    // Maintain keyboard compatibility (SPACE key)
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
        jumpButtonPressed = true;
    }

    // First jump when on ground
    if (jumpButtonPressed && !isJumping && !isFalling) {
        // Apply jump force immediately
        float jumpForce = 3.5f;

        // Apply impulse to both bodies
        pbodyUpper->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
        pbodyLower->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);

        // Set jump state
        isJumping = true;
        isFalling = false;
        jumpCount = 1;
        canDoubleJump = doubleJump; // Only enable double jump if the feature is active
        Engine::GetInstance().audio.get()->PlayFx(jumpFX);

        // Reset and start jump animation
        jump.Reset();
        currentAnimation = &jump;
        texture = jumpTexture;
    }
    // Double jump when already in air and double jump is enabled
    else if (jumpButtonPressed && (isJumping || isFalling) && canDoubleJump && jumpCount < 2) {
        // Cancel current vertical velocity for a consistent second jump height
        pbodyUpper->body->SetLinearVelocity(b2Vec2(pbodyUpper->body->GetLinearVelocity().x, 0));
        pbodyLower->body->SetLinearVelocity(b2Vec2(pbodyLower->body->GetLinearVelocity().x, 0));

        // Apply second jump force (can be different from the first jump if desired)
        float doubleJumpForce = 3.0f;

        pbodyUpper->body->ApplyLinearImpulseToCenter(b2Vec2(0, -doubleJumpForce), true);
        pbodyLower->body->ApplyLinearImpulseToCenter(b2Vec2(0, -doubleJumpForce), true);

        // Update state
        isJumping = true;
        isFalling = false;
        jumpCount = 2;
        canDoubleJump = false; // Disable further jumps until landing
        Engine::GetInstance().audio.get()->PlayFx(jumpFX);

        // Reset jump animation for the second jump
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

            float verticalVelocity = pbodyLower->body->GetLinearVelocity().y;
            bool onGround = fabs(verticalVelocity) < 0.1f;

            if (onGround) {
                timeNotGrounded = 0.0f;
            }
            else {
                timeNotGrounded += dt;

                if (timeNotGrounded >= 1000.0f && !isStrongFall) {
                    isStrongFall = true;

                }
            }

            // Check both bodies' vertical velocity to determine if player has actually stopped falling
            float upperVelocity = pbodyUpper->body->GetLinearVelocity().y;
            float lowerVelocity = pbodyLower->body->GetLinearVelocity().y;

            // If both bodies have very little vertical movement and the player isn't intentionally jumping
            if (fabs(upperVelocity) < 0.1f && fabs(lowerVelocity) < 0.1f && !isJumping) {
                // We've probably landed but the collision wasn't registered properly
                isJumping = false;
                isFalling = false;
                jumpCount = 0;      // Reset jump count on landing
                canDoubleJump = false; // Reset double jump ability until next initial jump
                fallingTimer = 0.0f;  // Reset the timer
                timeNotGrounded = 0.0f;
                // Transition to recovering animation
                isRecovering = true;
                if (isStrongFall) {
                    recoveringTimer = strongFallDurantion;
                    strongfall.Reset();
                }
                else {
                    recoveringTimer = recoveringDuration;
                    recovering.Reset();
                }
            }
        
    }
   }


void Player::HandleSceneSwitching() {
    // Level switching controls
    int currentLvl = Engine::GetInstance().sceneLoader->GetCurrentLevel();
    //Go to scene 1, Start of the game
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_1) == KEY_DOWN && currentLvl != 1 ) {
        //Move the player to the start position tunel scene
        Engine::GetInstance().sceneLoader->LoadScene(1, 2877, 2048, false, false);
    }
    //In Scene 1, pressing F8 will move the player to the dash item:
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F8) == KEY_DOWN) {
        int debugXLevelDesignInitialPositionOfLevel = 6600;
        int debugYLevelDesignInitialPositionOfLevel = 3990;
        Vector2D debugPos(debugXLevelDesignInitialPositionOfLevel, debugYLevelDesignInitialPositionOfLevel);
        SetPosition(debugPos);
    }
    //In Scene 1, pressing F7 will move the player to the connector door:
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) {
        int debugXLevelDesignInitialPositionOfLevel = 5380;
        int debugYLevelDesignInitialPositionOfLevel = 5970;
        Vector2D debugPos(debugXLevelDesignInitialPositionOfLevel, debugYLevelDesignInitialPositionOfLevel);
        SetPosition(debugPos);
    }
    //Go to Scene 2, Puzzle Scene
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_2) == KEY_DOWN && currentLvl != 2 ) {
        Engine::GetInstance().sceneLoader->LoadScene(2, 3097, 729, false, false);
    }
    //Go to Scene 3, First Boss Scene
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_3) == KEY_DOWN && currentLvl != 3 ) {
        Engine::GetInstance().sceneLoader->LoadScene(3, 700, 600, false, false);
    }
    //Go to scene 4, Double Jump item
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_4) == KEY_DOWN && currentLvl != 4 ) {
        Engine::GetInstance().sceneLoader->LoadScene(4, 724, 422, false, false);
    }
    //Go to Scene 5, Second Boss Scene
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_5) == KEY_DOWN && currentLvl != 5 ) {
        Engine::GetInstance().sceneLoader->LoadScene(5, 1248, 608, false, false);
    }
    //Go to Scene 6, Central Station
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_6) == KEY_DOWN && currentLvl != 6 ) {
        Engine::GetInstance().sceneLoader->LoadScene(6, 2180, 600, false, false);
    }
    //Go to Scene 7, Connector Right
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_7) == KEY_DOWN && currentLvl != 7 ) {
        Engine::GetInstance().sceneLoader->LoadScene(7, 4950, 3970, false, false);
    }
    //Go to Scene 8, Ice tunnel, VERY CLOSE TO THE DOOR OF THE CONNECTOR
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_8) == KEY_DOWN && currentLvl != 8 ) {
        Engine::GetInstance().sceneLoader->LoadScene(8, 8340, 2640, false, false);
    }
    //Go to Scene 9, Connector Left
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_9) == KEY_DOWN && currentLvl != 9 ) {
        Engine::GetInstance().sceneLoader->LoadScene(9, 4574, 2213, false, false);
    }
    //Go to Scene 10, Left Electric tunnel, VERY CLOSE TO THE DOOR OF THE CONNECTOR
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_T) == KEY_DOWN && currentLvl != 10 ) {
        Engine::GetInstance().sceneLoader->LoadScene(10, 4620, 2570, false, false);
    }
    //Go to Scene 11
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Y) == KEY_DOWN && currentLvl != 11 ) {
        Engine::GetInstance().sceneLoader->LoadScene(11, 9730, 1662, false, false);
    }
    //In Scene 11, pressing F6 will move the player to the connector door:
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
        int debugXLevelDesignInitialPositionOfLevel = 4600;
        int debugYLevelDesignInitialPositionOfLevel = 5500;
        Vector2D debugPos(debugXLevelDesignInitialPositionOfLevel, debugYLevelDesignInitialPositionOfLevel);
        SetPosition(debugPos);
    }
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_I) == KEY_DOWN && currentLvl != 12 ) {
        Engine::GetInstance().sceneLoader->LoadScene(12, 643, 1592, false, true);
    }
    static float zoom = 1.5f;
    static bool zooming = false;

    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Z) == KEY_DOWN) {
        zooming = true;
    }

    if (zooming) {
        zoom -= 0.01f; // velocidad del zoom
        if (zoom <= 1.0f) {
            zoom = 1.0f;
            zooming = false;
        }
        GlobalSettings::GetInstance().SetTextureMultiplier(zoom);
    }

    //Debug Mode:
        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_DOWN) {//Open Puzzle Doors
            Engine::GetInstance().scene->SetOpenDoors();
        }
        // unlocks sensors
        if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_0) == KEY_DOWN) {//unlocks sensors scene change
            DesbloquearSensor();
            Engine::GetInstance().entityManager->AscensorOn();
        }
}
void Player::HandleHurt(float dt) {
    if(!godMode){
    if (isHurtDelayed) {
        currentHurtDelay += dt;
       
        if (currentHurtDelay >= hurtDelay) {
            currentHurtDelay += dt;
            if (currentHurtDelay >= hurtDelay) {
                isHurt = true;
                isHurtDelayed = false;
                hurt.Reset();
                currentAnimation = &hurt;
                texture = hurtTexture;
                hasHurtStarted = true;
                if(ballhurt){
					lives -= 1;
                }
                else if(bufonjumphurt) {
                    lives -= 2;
                }
                else if (demopunch) {
                    lives -= 1;
                }
                else {
                    lives -= 2;
                }
                Engine::GetInstance().audio.get()->PlayFx(hurtFX);

            }
        }
        return; // Salimos para no procesar la animación hasta que termine el retraso
    }
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
			// reset to idle state
            isHurt = false;
            hasHurtStarted = false;
            hurted = false;
            freezeWhileHurting = false;
            ballhurt = false;
            bufonjumphurt = false;
            demopunch = false;
            currentHurtDelay = 0.0f;
            isHurtDelayed = false;
            currentAnimation = &idle;
            idle.Reset();
            hurt.Reset();
        }   
    }
    }
}

void Player::HandlePickup(float dt) {
    if (isPickingUp && !isDying) {
        if (!hasPickupStarted) {
            pickupAnim.Reset();
            hasPickupStarted = true; // Reset pickup animation one time
        }
        pickupAnim.Update(); 

        if (pickupAnim.HasFinished()) { 
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
        isHurt = false;
        hasHurtStarted = false;
        hurted = false;
		hasDeathStarted = false;
        hurt.Reset();
        ResetLives();
        isWakingUp = true;
		currentAnimation = &wakeupAnim;
		wakeupAnim.Reset();

        ResetPlayerPosition();
    }
}

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

    auto& engine = Engine::GetInstance();
    bool whipAttackButtonPressed = false;

    // Check controller input for whip attack (R2 trigger)
    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();
        // R2 is SDL_CONTROLLER_AXIS_TRIGGERRIGHT, check if pressed
        Sint16 rightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        // Consider it pressed when the trigger is over 1/4 of its range
        const int TRIGGER_THRESHOLD = 8000;  // 1/4 of the maximum range (32768)
        if (rightTrigger > TRIGGER_THRESHOLD) {
            whipAttackButtonPressed = true;
        }
    }

    // Maintain keyboard compatibility (K key)
    if (engine.input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN) {
        whipAttackButtonPressed = true;
    }

    // Initiate whip attack only when not dashing, melee attacking, or already whip attacking
    if (whipAttackButtonPressed && !isWhipAttacking && !isAttacking && !isDashing &&
        canWhipAttack && WhipAttack && !isHurt) {
        // Start whip attack
        engine.audio.get()->PlayFx(whipFX);
        isWhipAttacking = true;
        canWhipAttack = false;
        whipAttackCooldown = 0.7f;
        // Reset the animation instead of recreating it
        whipAttack.Reset();
        // Create whip attack hitbox
        int attackWidth = texW * 2.5;
        int attackHeight = texH / 2;
        b2Vec2 playerCenter = pbodyUpper->body->GetPosition();  // Use upper body for attacks
        int centerX = METERS_TO_PIXELS(playerCenter.x);
        int centerY = METERS_TO_PIXELS(playerCenter.y);
        // Calculate attack position based on facing direction
        int attackX = facingRight ? centerX + texW / 4 : centerX - texW / 4 - attackWidth / 2;
        // Remove existing whip attack hitbox
        if (whipAttackHitbox) {
            engine.physics.get()->DeletePhysBody(whipAttackHitbox);
            whipAttackHitbox = nullptr;
        }
        // Create new whip attack hitbox
        whipAttackHitbox = engine.physics.get()->CreateRectangleSensor(
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
            int attackX = (int)position.getX() + (facingRight ? 128 : -28);
            int attackY = position.getY() + 10;
            whipAttackHitbox->body->SetTransform({ PIXEL_TO_METERS(attackX), PIXEL_TO_METERS(attackY) }, 0);
        }
        // Check if animation finished
        if (whipAttack.HasFinished()) {
            isWhipAttacking = false;
            currentAnimation = &idle;  // Explicitly switch back to idle animation
            texture = idleTexture;
            if (whipAttackHitbox) {
                engine.physics.get()->DeletePhysBody(whipAttackHitbox);
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

    auto& engine = Engine::GetInstance();
    bool attackButtonPressed = false;

    // Check controller input for melee attack (X/Square button)
    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();
        // SDL_CONTROLLER_BUTTON_X corresponds to the left button (Square on PlayStation, X on Xbox)
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X)) {
            attackButtonPressed = true;
        }
    }

    // Maintain keyboard compatibility (J key)
    if (engine.input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
        attackButtonPressed = true;
    }

    // Initiate melee attack only when not dashing, whip attacking, or already attacking
    if (attackButtonPressed && canAttack && !isAttacking && !isWhipAttacking && !isDashing && !isHurt) {

        LOG("Golpisa");

        Engine::GetInstance().audio.get()->PlayFx(punchFX);

        isAttacking = true;
        canAttack = false;           
        attackCooldown = 500.0f;       
        meleeAttack.Reset();          

        // Calculate the size of the hitbox to fit the attack sprite
        int attackWidth = texW * 0.5f;
        int attackHeight = texH * 1.0f;

        b2Vec2 playerCenter = pbodyUpper->body->GetPosition(); // Use upper body
        int centerX = METERS_TO_PIXELS(playerCenter.x);
        int centerY = METERS_TO_PIXELS(playerCenter.y);

        int attackX = facingRight
            ? centerX + texW / 3 - attackWidth / -3  // right
            : centerX - texW / 3 - attackWidth / 3;  // left

        int attackY = centerY + texH / 7; // Move hitbox downward by part of the texture height

        if (attackHitbox) {
            Engine::GetInstance().physics.get()->DeletePhysBody(attackHitbox);
            attackHitbox = nullptr;
        }

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

            int attackX = facingRight
                ? centerX + texW / 3 - attackHitbox->width / -3
                : centerX - texW / 3 - attackHitbox->width / 3;

            int attackY = centerY + texH / 7;

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

void Player::HandleBallAttack(float dt) {
    if (ballCounter < 3) {
        ballCooldown -= dt;
        if (ballCooldown <= 0.0f) {
            ballCounter++;
            ballCooldown = 3000.0f;
        }
    }
    auto& engine = Engine::GetInstance();
    bool ballAttackButtonPressed = false;

    // Check controller input for ball attack (L2 trigger)
    if (engine.IsControllerConnected()) {
        SDL_GameController* controller = engine.GetGameController();
        // L2 is SDL_CONTROLLER_AXIS_TRIGGERLEFT, check if it's pressed
        Sint16 leftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        // Consider pressed when the trigger is more than 1/4 pressed
        const int TRIGGER_THRESHOLD = 8000;  // 1/4 of the max range (32768)

        // Store previous trigger state to detect when it is newly pressed
        static Sint16 previousLeftTrigger = 0;
        if (leftTrigger > TRIGGER_THRESHOLD && previousLeftTrigger <= TRIGGER_THRESHOLD) {
            ballAttackButtonPressed = true;
        }
        previousLeftTrigger = leftTrigger;
    }

    // Maintain keyboard compatibility (L key) - no changes
    if (engine.input.get()->GetKey(SDL_SCANCODE_L) == KEY_DOWN) {
        ballAttackButtonPressed = true;
    }

    // Add check for ball attack animation completion before allowing next shot
    if (ballAttackButtonPressed && engine.scene.get()->normalProjectileConfigNode && ballCounter > 0 && BallAttack) {
        ballCounter--;
        isThrowing = true;
        hasThrownBall = false;
        ballToShoot = true;
        throwAnim.Reset();
        currentAnimation = &throwAnim;
        texture = throwTexture;
    }

    if (isThrowing && ballToShoot) {
        int currentFrame = throwAnim.GetCurrentFrameIndex(); 

        const int THROW_FRAME = 5; 

        if (currentFrame == THROW_FRAME && !hasThrownBall) {

            Projectiles* projectile = (Projectiles*)engine.entityManager->CreateEntity(EntityType::PROJECTILE);
            projectile->SetParameters(engine.scene.get()->normalProjectileConfigNode);
            projectile->Start();
            Vector2D playerPosition = engine.scene.get()->GetPlayerPosition();
            if (!facingRight) playerPosition.setX(playerPosition.getX() - 50);
            else playerPosition.setX(playerPosition.getX() + 20);
            projectile->SetPosition(playerPosition);
            projectile->SetDirection(facingRight);

            hasThrownBall = true;  
        }

        if (throwAnim.HasFinished()) {
            isThrowing = false;
            ballToShoot = false;
            hasThrownBall = false;
            currentAnimation = &idle;
            texture = idleTexture;
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
        
        // Check if the animation finished and we were waiting for teleport
        if (hurt.HasFinished() && waitForHurtAnimation) {
            isHurt = false;
        }
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
        if (isStrongFall) {
            currentAnimation = &strongfall;
            texture = strongfallTexture;
            strongfall.Update();
        }
        else {
            currentAnimation = &recovering;
            texture = recoveringTexture;
            recovering.Update();
        }

        recoveringTimer -= 0.0167f;
        if (recoveringTimer <= 0 ||
            (isStrongFall && strongfall.HasFinished()) ||
            (!isStrongFall && recovering.HasFinished())) {
            isRecovering = false;
            isStrongFall = false;
            timeNotGrounded = 0.0f;
            currentAnimation = &idle;
            idle.Reset();
        }
    }
    else if (isWalking) {
      
            currentAnimation = &walk;
            texture = walkTexture;
            walk.Update();
        

        if (!resbalar && !isJumping && isWalking && !isDashing) {
            runSoundTimer += 0.0167;
            if (runSoundTimer >= runSoundInterval) {
                Engine::GetInstance().audio.get()->PlayFx(stepFX);
                runSoundTimer = 0.0f;
            }
        }
    }
    else if (currentAnimation == &throwAnim) {
        throwAnim.Update();
        if (throwAnim.HasFinished()) {
            currentAnimation = &idle;
            texture = idleTexture;
        }
    }
    else {
        if (resbalar) {
            currentAnimation = &slide;
            texture = slideTexture;
            slide.Update();

        }
        else {

            (currentAnimation != &idle);
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
    if (isAttacking)
    {
        if (facingRight) {

            drawX = position.getX() +1;
            drawY = position.getY() -16;
        }
        else {

            drawX = position.getX() - 2.25 ;
            drawY = position.getY() - 16;

        }
    }
    if (isRecovering && isStrongFall) {
        if (facingRight) {

            drawX = position.getX() + 18;
            drawY = position.getY() - 15;
        }
        else {
            drawX = position.getX() + 12;
            drawY = position.getY() - 15;
        }
    }

    if (isJumping || isFalling || isRecovering && !isStrongFall) {
        if (facingRight) {

            drawX = position.getX() - 7 ;
            drawY = position.getY() - 10;
        }
        else {

            drawX = position.getX() +  7;
            drawY = position.getY() - 10 ;

        }
    }
    if (currentAnimation == &throwAnim) {
        if (facingRight) {
            drawX = position.getX() - 7;  
        }
        else {
            drawX = position.getX() + 7;  
          
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

void Player::ResetToInitPosition() {
    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node playerNode;

    for (int i = 0; i < 11; ++i) {
        if (i == 0) {
            playerNode = doc.child("config")
                .child("scene")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene1InitX);
            playerNode.attribute("y").set_value(Scene1InitY);
        }
        else if (i == 1) {
            playerNode = doc.child("config")
                .child("scene2")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene2InitX);
            playerNode.attribute("y").set_value(Scene2InitY);
        }
        else if (i == 2) {
            playerNode = doc.child("config")
                .child("scene3")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene3InitX);
            playerNode.attribute("y").set_value(Scene3InitY);
        }
        else if (i == 3) {
            playerNode = doc.child("config")
                .child("scene4")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene4InitX);
            playerNode.attribute("y").set_value(Scene4InitY);
        }
        else if (i == 4) {
            playerNode = doc.child("config")
                .child("scene5")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene5InitX);
            playerNode.attribute("y").set_value(Scene5InitY);
        }
        else if (i == 5) {
            playerNode = doc.child("config")
                .child("scene6")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene6InitX);
            playerNode.attribute("y").set_value(Scene6InitY);
        }
        else if (i == 6) {
            playerNode = doc.child("config")
                .child("scene7")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene7InitX);
            playerNode.attribute("y").set_value(Scene7InitY);
        }
        else if (i == 7) {
            playerNode = doc.child("config")
                .child("scene8")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene8InitX);
            playerNode.attribute("y").set_value(Scene8InitY);
        }
        else if (i == 8) {
            playerNode = doc.child("config")
                .child("scene9")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene9InitX);
            playerNode.attribute("y").set_value(Scene9InitY);
        }
        else if (i == 9) {
            playerNode = doc.child("config")
                .child("scene10")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene10InitX);
            playerNode.attribute("y").set_value(Scene10InitY);
        }
        else if (i == 10) {
            playerNode = doc.child("config")
                .child("scene11")
                .child("entities")
                .child("player");
            playerNode.attribute("x").set_value(Scene11InitX);
            playerNode.attribute("y").set_value(Scene11InitY);
        }
    }

    if (!playerNode) {
        LOG("Could not find the node for player in the XML");
        return;
    }

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for player");
    }

}
void Player::SaveInitialPosition() {

    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file("config.xml")) {
        LOG("Error loading config.xml");
        return;
    }

    pugi::xml_node playerNode;

    for (int i = 0; i < 11; ++i) {
        if (i == 0) {
            playerNode = doc.child("config")
                .child("scene")
                .child("entities")
                .child("player");
            Scene1InitX = playerNode.attribute("x").as_float();
            Scene1InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 1) {
            playerNode = doc.child("config")
                .child("scene2")
                .child("entities")
                .child("player");
            Scene2InitX = playerNode.attribute("x").as_float();
            Scene2InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 2) {
            playerNode = doc.child("config")
                .child("scene3")
                .child("entities")
                .child("player");
            Scene3InitX = playerNode.attribute("x").as_float();
            Scene3InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 3) {
            playerNode = doc.child("config")
                .child("scene4")
                .child("entities")
                .child("player");
            Scene4InitX = playerNode.attribute("x").as_float();
            Scene4InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 4) {
            playerNode = doc.child("config")
                .child("scene5")
                .child("entities")
                .child("player");
            Scene5InitX = playerNode.attribute("x").as_float();
            Scene5InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 5) {
            playerNode = doc.child("config")
                .child("scene6")
                .child("entities")
                .child("player");
            Scene6InitX = playerNode.attribute("x").as_float();
            Scene6InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 6) {
            playerNode = doc.child("config")
                .child("scene7")
                .child("entities")
                .child("player");
            Scene7InitX = playerNode.attribute("x").as_float();
            Scene7InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 7) {
            playerNode = doc.child("config")
                .child("scene8")
                .child("entities")
                .child("player");
            Scene8InitX = playerNode.attribute("x").as_float();
            Scene8InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 8) {
            playerNode = doc.child("config")
                .child("scene9")
                .child("entities")
                .child("player");
            Scene9InitX = playerNode.attribute("x").as_float();
            Scene9InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 9) {
            playerNode = doc.child("config")
                .child("scene10")
                .child("entities")
                .child("player");
            Scene10InitX = playerNode.attribute("x").as_float();
            Scene10InitY = playerNode.attribute("y").as_float();
        }
        else if (i == 10) {
            playerNode = doc.child("config")
                .child("scene11")
                .child("entities")
                .child("player");
            Scene11InitX = playerNode.attribute("x").as_float();
            Scene11InitY = playerNode.attribute("y").as_float();
        }
    }

    if (!playerNode) {
        LOG("Could not find the node for player in the XML");
        return;
    }

    if (!doc.save_file("config.xml")) {
        LOG("Error saving config.xml");
    }
    else {
        LOG("death status updated in the XML for player");
    }

}

void Player::ResetPlayerPosition() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    if (result == NULL) {
        LOG("Could not load file. Pugi error: %s", result.description());
        return;
    }

    // Obtener la escena del último checkpoint activado
    int checkpointScene = GetLastCheckpointScene();
    int currentScene = Engine::GetInstance().sceneLoader.get()->GetCurrentLevel();

    if (checkpointScene == -1) {
        // Si no hay checkpoint guardado, usar la escena actual
        checkpointScene = currentScene;
    }

    // Obtener la posición guardada del checkpoint
    pugi::xml_node sceneNode;

    if (checkpointScene == 1) {
        sceneNode = loadFile.child("config").child("scene");
    }
    else if (checkpointScene == 2) {
        sceneNode = loadFile.child("config").child("scene2");
    }
    else if (checkpointScene == 3) {
        sceneNode = loadFile.child("config").child("scene3");
    }
    else if (checkpointScene == 4) {
        sceneNode = loadFile.child("config").child("scene4");
    }
    else if (checkpointScene == 5) {
        sceneNode = loadFile.child("config").child("scene5");
    }
    else if (checkpointScene == 6) {
        sceneNode = loadFile.child("config").child("scene6");
    }
    else if (checkpointScene == 7) {
        sceneNode = loadFile.child("config").child("scene7");
    }
    else if (checkpointScene == 8) {
        sceneNode = loadFile.child("config").child("scene8");
    }
    else if (checkpointScene == 9) {
        sceneNode = loadFile.child("config").child("scene9");
    }
    else if (checkpointScene == 10) {
        sceneNode = loadFile.child("config").child("scene10");
    }
    else if (checkpointScene == 11) {
        sceneNode = loadFile.child("config").child("scene11");
    }

    float x = sceneNode.child("entities").child("player").attribute("x").as_float();
    float y = sceneNode.child("entities").child("player").attribute("y").as_float();

    // Si la escena del checkpoint es diferente a la actual, cambiar de escena
    if (checkpointScene != currentScene) {
        LOG("Moving player from scene %d to checkpoint scene %d", currentScene, checkpointScene);
        Engine::GetInstance().sceneLoader.get()->LoadScene(checkpointScene, (int)x, (int)y, true, false);
    }
    else {
        // Si es la misma escena, solo reposicionar
        Vector2D newPos(x, y);
        SetPosition(newPos);
    }
}

// Nueva función para obtener la escena del último checkpoint
int Player::GetLastCheckpointScene() {
    pugi::xml_document loadFile;
    pugi::xml_parse_result result = loadFile.load_file("config.xml");
    if (result == NULL) {
        LOG("Could not load file. Pugi error: %s", result.description());
        return -1;
    }

    pugi::xml_node gameStateNode = loadFile.child("config").child("gamestate");
    if (!gameStateNode) {
        return -1;
    }

    pugi::xml_node lastCheckpointSceneNode = gameStateNode.child("lastCheckpointScene");
    if (!lastCheckpointSceneNode) {
        return -1;
    }

    pugi::xml_node sceneValueNode = lastCheckpointSceneNode.child("scene");
    if (!sceneValueNode) {
        return -1;
    }

    return sceneValueNode.attribute("value").as_int();
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
    // Logic so that the player cannot shoot if he has the enemy next to him
    if (physA == pbodyLower) {
        if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::TERRESTRE) {
            collidingWithEnemy = true;
            isTouchingEnemy = true;
            tocado = true;
            return;
        }
    }
    if (physA == pbodyUpper) {
        if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::VOLADOR) {
            collidingWithEnemy = true;
            isTouchingEnemy = true;
            tocado = true;
            return;
        }
    }
    if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::BOSS) {
        collidingWithEnemy = true;
        return;
    }
    if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::AMEGO) {
        collidingWithEnemy = true;
        return;
    }
    // End of the logic

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

    //if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::CHECKPOINT) {
    //    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
    //        ResetLives();
    //    } 
    //    return;
    //}

    if (physA->ctype == ColliderType::PLAYER  &&  physB->ctype == ColliderType::ITEM) {
        Item* item = (Item*)physB->listener;

        if (item) {

            isPickingUp = true;

            if (item && item->GetItemType() == "Dash ability") {
                Dash = true;
                Engine::GetInstance().audio.get()->PlayFx(itemFX);

            }
			if (item && item->GetItemType() == "Double jump") {
				doubleJump = true;
				Engine::GetInstance().audio.get()->PlayFx(itemFX);
			}
            if (item && item->GetItemType() == "Whip") {
                WhipAttack = true;
                Engine::GetInstance().audio.get()->PlayFx(itemFX);
                NeedDialogue = true; //activate dialog when touching item, in the xml put the id of the dialog to be activated
                Id = physB->ID; //ID from Item

            }
            if (item && item->GetItemType() == "Remember1") {
                Engine::GetInstance().audio.get()->PlayFx(itemFX);
                NeedDialogue = true; //activate dialog when touching item, in the xml put the id of the dialog to be activated
                Id = physB->ID; //ID from Item
               if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 1) {
                    Engine::GetInstance().mapa.get()->remember1 = true;
                }
                if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 3) {
                    Engine::GetInstance().mapa.get()->remember2 = true;
                }
                if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 4) {
                    Engine::GetInstance().mapa.get()->remember3 = true;
                }
                if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 6) {
                    Engine::GetInstance().mapa.get()->remember4 = true;
                }
                if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 8) {
                    if (Id == "1") {
                        Engine::GetInstance().mapa.get()->remember5 = true;
                    }
                    if (Id == "2") {
                        Engine::GetInstance().mapa.get()->remember6 = true;
                    }                    
                } 
                if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 10) {
                    Engine::GetInstance().mapa.get()->remember7 = true;
                }
                if (Engine::GetInstance().sceneLoader.get()->GetCurrentLevel() == 11) {
                    Engine::GetInstance().mapa.get()->remember8 = true;
                }
                
            }
            if (item && item->GetItemType() == "Door key") {
                canOpenDoor = true;
                Engine::GetInstance().audio.get()->PlayFx(itemFX);
                changeMusicCaronte = true; 
                //Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/Background.ogg", 1.0f);

            }
            if (item && item->GetItemType() == "Ball") {
				BallAttack = true;
                Engine::GetInstance().audio.get()->PlayFx(itemFX);
            }
        }
        return;
    }

    //if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::CHECKPOINT) {
    //    Checkpoints* checkpoint = (Checkpoints*)physB->listener;

    //    if (checkpoint) {

    //        if (checkpoint && checkpoint->GetCheckpointType() == "checkpoint1" && !checkpoint->GetActivitatedXML()) { 
    //            Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
    //            LOG("A");
    //            return;
    //        }
    //    }
    //    return;
    //}

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
                jumpCount = 0;         // Reset jump count on landing
                canDoubleJump = false; // Reset double jump ability
                fallingTimer = 0.0f;   // Reset the timer

                // Transition directly to recovering animation
                isRecovering = true;
                recoveringTimer = recoveringDuration;
                recoveringTimer = strongFallDurantion;
                strongfall.Reset();
                recovering.Reset();

                // Play the landing sound
                Engine::GetInstance().audio.get()->PlayFx(fallFX);
            }
        }
        break;
    case ColliderType::BOSS_ATTACK:
        if(!godMode){
            if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
                isHurtDelayed = true;
                currentHurtDelay = 0.0f;
            //freezeWhileHurting = true;

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
        }
        break;

    case ColliderType::DEVIL_PUNCH_ATTACK1:
        if (!godMode) {
            if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
                isHurtDelayed = true;
                currentHurtDelay = 0.0f;
                demopunch = true;
                //freezeWhileHurting = true;

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
        }
        break;

    case ColliderType::PROJECTILE:

        if (Engine::GetInstance().entityManager->BBuffon) {//Stop the bullets from hurting the player when the boss is dead
            if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
                isHurtDelayed = true;
                currentHurtDelay = 0.0f;
                freezeWhileHurting = true;
                ballhurt = true;

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
        }
        break;

    case ColliderType::BUFON_JUMP_ATTACK_AREA:
        if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
            isHurtDelayed = true;
            currentHurtDelay = 0.0f;
            freezeWhileHurting = true;
            bufonjumphurt = true;

			// Apply knockback force to the player
            float knockbackForce = facingRight ? 30.0f : 50.0f; 
            b2Vec2 impulse = facingRight ? b2Vec2(-knockbackForce, 0) : b2Vec2(knockbackForce, 0);

            pbodyUpper->body->ApplyLinearImpulseToCenter(impulse, true);
            pbodyLower->body->ApplyLinearImpulseToCenter(impulse, true);

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

    case ColliderType::LEVER: {
        break;
    }
    case ColliderType::MOSAIC_PIECE: {
         break;
    }
    case ColliderType::MOSAIC_LEVER: {
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
    case ColliderType::PLATFORMICE:
        resbalar = true;//Ice platform
        break;
    case ColliderType::ABYSS:
        if (!isFallingInAbyss && !godMode) {
            if (canHurtAbyss) {
                if (!godMode) {
                    lives--;
                }
            }
            canHurtAbyss = false;
            isFallingInAbyss = true;
        }
        break;
    case ColliderType::UNKNOWN:
        break;
    }
}

void Player::Ascensor() {
    bool useElevator = false;

    // Original keyboard input
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
        useElevator = true;
    }

    // Add support for controller Circle button (B in SDL terminology)
    if (Engine::GetInstance().IsControllerConnected()) {
        SDL_GameController* controller = Engine::GetInstance().GetGameController();
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) == KEY_DOWN) {
            useElevator = true;
        }
    }

    if (useElevator && TocandoAs == true) {
        NeedSceneChange = true;
    }
}
void Player::Abyss()
{
    if (isFallingInAbyss) {
        if (!waitForHurtAnimation) {
            // Set hurt state without teleporting yet
            isHurt = true;
            hasHurtStarted = true;
            waitForHurtAnimation = true;
            pendingAbyssTeleport = true;
            // Play hurt sound if needed
            Engine::GetInstance().audio.get()->PlayFx(hurtFX);
        }
        else if (pendingAbyssTeleport && !isHurt) {
            // The hurt animation has finished, now teleport
            Player* player = Engine::GetInstance().scene->GetPlayer();
            player->SetPosition(Vector2D(abyssTeleportX, abyssTeleportY));
            canHurtAbyss = true;
            // Reset velocities
            pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0));
            pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));
            // Reset states
            waitForHurtAnimation = false;
            pendingAbyssTeleport = false;
            isFallingInAbyss = false; // Make sure to reset this flag
            hurted = false;
        }
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
    if (physA == pbodyLower) {
        if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::TERRESTRE) {
            collidingWithEnemy = false;
            isTouchingEnemy = false;
            //first = true;
            tocado = false;
            return;
        }
    }
    if (physA == pbodyUpper) {
        if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::VOLADOR) {
            collidingWithEnemy = false;
            isTouchingEnemy = false;
            tocado = false;
            return;
        }
    }
    if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::BOSS) {
        collidingWithEnemy = false;
        return;
    }
    if (physA->ctype == ColliderType::PLAYER && physB->ctype == ColliderType::AMEGO) {
        collidingWithEnemy = false;
        if (isHurt) {
            isHurt = false;
            hasHurtStarted = false;
            hurted = false;
        }
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
            freezeWhileHurting = false;
        }
        break;
    case ColliderType::PROJECTILE:
        break;
    case ColliderType::BUFON_JUMP_ATTACK_AREA:
        break;
    case ColliderType::ASCENSORES:
        TocandoAs = false;
        break;
    case ColliderType::DIALOGOS:
        physB->Salio = true;
        break;
    case ColliderType::PLATFORMICE:
        resbalar = false;//Ice platform
        break;

    
    case ColliderType::ABYSS:
        hurt.Reset();
         break;
     }
}


void Player::SetPosition(Vector2D pos) {
    int adjustment = 2;
    // Establish upper body position
    pos.setX(pos.getX() );
    pos.setY(pos.getY() + texH /3 + adjustment);  
    b2Vec2 upperPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbodyUpper->body->SetTransform(upperPos, 0);

    // Establish lower body position
    Vector2D lowerPos = pos;
    lowerPos.setY(pos.getY() + texH / 3 + adjustment);  
    b2Vec2 lowerPosB2 = b2Vec2(PIXEL_TO_METERS(lowerPos.getX()), PIXEL_TO_METERS(lowerPos.getY()));
    pbodyLower->body->SetTransform(lowerPosB2, 0);
}

Vector2D Player::GetPosition() {
    int adjustment = 2;

    // Use upper body position as a reference
    b2Vec2 bodyPos = pbodyUpper->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	pos.setY(pos.getY() - texH / 3 + adjustment);  
    return pos;
}
void Player::hit(){
    if (!godMode) {
        isHurt = true;
        lives--;
    }
}

void Player::HitWcooldown(float dt) {

    if (tocado) {
        if (first) {
            if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
                hit();
                first = false;
            }
        }
        else {
            hitCooldown -= dt;
            if (hitCooldown <= 0.0f) {
                if (!isHurt && !hasHurtStarted && lives > 0 && !isDying) {
                    hit();
                    hitCooldown = 2000.0f;
                }
                //hurted = false;
                //isHurt = false;
            }    
          // if(isHurt && !hasHurtStarted && lives > 0 && !isDying && hurted){ hurt.Reset(); isHurt = false;  hurted = false;}         
            if (hurt.HasFinished() && hurted) {
                // Reset to idle
                hurted = false;
                isHurt = false;
                hasHurtStarted = false;
            }
        }
    }
    else {
      // first = true;
        /*hurted = false;
        isHurt = false;
        hasHurtStarted = false;*/
       // hurted = false;
       // if (isHurt && !hasHurtStarted && lives > 0 && !isDying && hurted) { hurt.Reset(); isHurt = false;  hurted = false; }
    }
   
    
    
    
    
    
    /* // update latest status
    wasTouchingEnemy = isTouchingEnemy;

    // Reduce cooldown
    if (damageCooldown > 0.0f) {
        damageCooldown -= dt;
    }
    if (isTouchingEnemy && damageCooldown <= 0.0f && !isHurt && !isDying) {
        hit();
        damageCooldown = DAMAGE_COOLDOWN_TIME;
    }
    //Reset status if contact is lost
    if (!isTouchingEnemy && wasTouchingEnemy) {
        damageCooldown = 0.0f;
    }*/
}

void Player::RestoreFullStats() {
    
    lives = 5;
}

