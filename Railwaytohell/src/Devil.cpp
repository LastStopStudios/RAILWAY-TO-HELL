#include "Devil.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "EntityManager.h"

Devil::Devil() : Entity(EntityType::DEVIL)
{
    currentPhase = 1;
    lives = 3; // 3 lives for 3 phases
    live1 = 1; // Lives for phase 1
    live2; // Lives for phase 2
    live3; // Lives for phase 3
}

Devil::~Devil() {
    if (punchAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
    }
    if (jumpAttackArea != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
    }
}

bool Devil::Awake() {
    return true;
}

bool Devil::Start() {
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    shadowTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/bosses/Shadow.png");
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    initX = parameters.attribute("x").as_int();
    initY = parameters.attribute("y").as_int();

    // Load all animations from XML parameters
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    walk.LoadAnimations(parameters.child("animations").child("walk"));
    punch.LoadAnimations(parameters.child("animations").child("punch"));
    defeat.LoadAnimations(parameters.child("animations").child("defeat"));
    transform.LoadAnimations(parameters.child("animations").child("transform"));
    idle2.LoadAnimations(parameters.child("animations").child("idle2"));
    salto.LoadAnimations(parameters.child("animations").child("salto"));
    land.LoadAnimations(parameters.child("animations").child("land"));
    colatazo.LoadAnimations(parameters.child("animations").child("colatazo"));
    transform2.LoadAnimations(parameters.child("animations").child("transform2"));
    idle3.LoadAnimations(parameters.child("animations").child("idle3"));
    attack.LoadAnimations(parameters.child("animations").child("attack"));
    currentAnimation = &idle;

    moveSpeed = 2.0f;
    patrolSpeed = 3.0f;

    // Create circular physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW / 2 - 3,
        bodyType::DYNAMIC
    );

    pbody->ctype = ColliderType::DEVIL;
    pbody->body->SetGravityScale(10.0f);

    pathfinding = new Pathfinding();

    return true;
}

bool Devil::Update(float dt) {
    bool isGameplay = Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY;

    // Pause physics during non-gameplay states
    if (!isGameplay) {
        if (pbody && pbody->body) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }
        return true;
    }
    else {
        if (pbody && pbody->body) {
            pbody->body->SetGravityScale(1.0f);
        }
    }

    // Handle transformation state
    if (isTransforming) {
        HandleTransformation(dt);
        UpdatePosition();
        RenderSprite();
        return true;
    }

    // Handle jump attack state
    if (jumpAttackActive) {
        UpdateJumpAttack(dt);
        UpdatePosition();
        RenderSprite();
        if (shadowVisible) {
            RenderShadow();
        }
        currentAnimation->Update();
        return true;
    }

    // Calculate distances for AI behavior
    enemyPos = GetPosition();
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    float dx = playerTilePos.getX() - enemyTilePos.getX();
    float distanceToPlayer = abs(dx);
    isLookingLeft = dx < 0;

    // Phase-based behavior
    switch (currentPhase) {
    case 1:
        HandlePhase1(distanceToPlayer, dx, dt);
        break;
    case 2:
        HandlePhase2(distanceToPlayer, dx, dt);
        break;
    case 3:
        HandlePhase3(distanceToPlayer, dx, dt);
        break;
    default:
        LOG("Error: Invalid phase %d", currentPhase);
        break;
    }

    UpdatePosition();
    RenderSprite();
    if (shadowVisible) {
        RenderShadow();
    }
    currentAnimation->Update();

    // Debug pathfinding visualization
    if (isChasing && !pathfinding->pathTiles.empty()) {
        pathfinding->DrawPath();
    }

    // Update UI health display
    if (Engine::GetInstance().ui->figth3 == true) {
        if (currentPhase == 1) { Engine::GetInstance().ui->vidab3 = live1; }
        if (currentPhase == 2) { Engine::GetInstance().ui->vidab3 = lives; }
        if (currentPhase == 3) { Engine::GetInstance().ui->vidab3 = lives; }
    }
    return true;
}

void Devil::HandlePhase1(float distanceToPlayer, float dx, float dt) {
    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    const float DETECTION_DISTANCE = 12.0f;
    const float CHASE_SPEED = moveSpeed;
    const int MAX_PATHFINDING_ITERATIONS = 50;

    // Handle punch attack animation
    if (isAttacking && currentAnimation == &punch) {
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));

        if (currentAnimation->HasFinished()) {
            isAttacking = false;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation = &idle;

            if (punchAttackArea) {
                Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                punchAttackArea = nullptr;
            }
            punch.Reset();
        }
    }
    else if (!isAttacking) {
        // Update attack cooldown
        if (!canAttack) {
            currentAttackCooldown -= dt;
            if (currentAttackCooldown <= 0) {
                canAttack = true;
                currentAttackCooldown = 0.0f;
            }
        }

        // Attack if close enough and ready
        if (distanceToPlayer <= 3.0f && canAttack) {
            CreatePunchAttack();
        }
        // Chase player using pathfinding
        else if (distanceToPlayer <= DETECTION_DISTANCE) {
            isChasing = true;

            Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
            Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
            Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

            ResetPath();

            // Calculate path to player
            for (int i = 0; i < MAX_PATHFINDING_ITERATIONS; i++) {
                pathfinding->PropagateAStar(SQUARED);
                if (pathfinding->ReachedPlayer(playerTilePos)) {
                    break;
                }
            }

            pathfinding->ComputePath(playerTilePos.getX(), playerTilePos.getY());
            currentAnimation = &walk;

            // Move towards next tile in path
            if (!pathfinding->pathTiles.empty() && pathfinding->pathTiles.size() > 1) {
                Vector2D nextTile = *(std::next(pathfinding->pathTiles.begin(), 1));
                Vector2D nextPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

                float moveX = nextPos.getX() - enemyPos.getX();

                if (moveX < -1.0f) isLookingLeft = true;
                else if (moveX > 1.0f) isLookingLeft = false;

                float velocityX = isLookingLeft ? -CHASE_SPEED : CHASE_SPEED;

                if (fabs(moveX) < 5.0f) velocityX *= 0.5f;

                pbody->body->SetLinearVelocity(b2Vec2(velocityX, pbody->body->GetLinearVelocity().y));
            }
            else {
                float direction = isLookingLeft ? -1.0f : 1.0f;
                pbody->body->SetLinearVelocity(b2Vec2(direction * moveSpeed, pbody->body->GetLinearVelocity().y));
            }
        }
        // Return to idle if player is far away
        else {
            isChasing = false;
            currentAnimation = &idle;
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        }
    }
}

void Devil::HandlePhase2(float distanceToPlayer, float dx, float dt) {
    const float JUMP_ATTACK_DISTANCE = 8.0f;

    // Update attack cooldown
    if (!canAttack) {
        currentAttackCooldown -= dt;
        if (currentAttackCooldown <= 0) {
            canAttack = true;
            currentAttackCooldown = 0.0f;
        }
    }

    // Jump attack if player is within range
    if (distanceToPlayer <= JUMP_ATTACK_DISTANCE && canAttack && !jumpAttackActive) {
        CreateJumpAttack();
    }
    else if (!jumpAttackActive) {
        currentAnimation = &idle2;
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
    }
}

void Devil::HandlePhase3(float distanceToPlayer, float dx, float dt) {
    // Phase 3 placeholder - currently just idle
    currentAnimation = &idle2;
    pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
}

void Devil::CreateJumpAttack() {
    jumpAttackActive = true;
    isJumping = true;
    isLanding = false;
    canAttack = false;
    hasReachedPeak = false;
    startFalling = false;

    // Get player position for targeting
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D currentPos = GetPosition();

    jumpStartPos = currentPos;
    targetLandingPos = playerPos;
    targetPlayerX = playerPos.getX(); // Store initial player X position

    // Set shadow at target position
    shadowPosition = targetLandingPos;
    shadowVisible = true;

    currentAnimation = &salto;
    currentAnimation->Reset();

    // Launch with horizontal velocity towards player
    float horizontalDirection = (targetPlayerX > currentPos.getX()) ? 1.0f : -1.0f;
    float horizontalSpeed = 8.0f;

    pbody->body->SetLinearVelocity(b2Vec2(horizontalDirection * horizontalSpeed, -18.0f));
    pbody->body->SetGravityScale(1.0f);
}

void Devil::UpdateJumpAttack(float dt) {
    Vector2D currentPos = GetPosition();
    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

    // Update shadow position
    if (shadowVisible) {
        shadowPosition.setX(currentPos.getX());
    }

    // Check if reached peak of jump
    if (!hasReachedPeak && currentVelocity.y >= 0) {
        hasReachedPeak = true;
        // Fix target X position at peak
        Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
        targetPlayerX = playerPos.getX();
    }

    // If at peak, check if reached target X position
    if (hasReachedPeak && !startFalling) {
        float distanceToPlayerX = abs(currentPos.getX() - targetPlayerX);

        if (distanceToPlayerX < 20.0f) {
            // Start diving down
            startFalling = true;
            pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            pbody->body->SetGravityScale(5.0f);
            currentAnimation = &land;
            currentAnimation->Reset();
        }
        else {
            // Continue moving horizontally
            float horizontalDirection = (targetPlayerX > currentPos.getX()) ? 1.0f : -1.0f;
            pbody->body->SetLinearVelocity(b2Vec2(horizontalDirection * 3.0f, 0.0f));
            pbody->body->SetGravityScale(0.0f);
        }
    }

    // Maintain vertical dive
    if (startFalling) {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, 30.0f));
    }

    // Check for landing
    if (hasReachedPeak && currentPos.getY() >= jumpStartPos.getY() - 10) {
        // Correct position if overshot
        if (currentPos.getY() > jumpStartPos.getY()) {
            b2Vec2 correctedPos = pbody->body->GetPosition();
            correctedPos.y = PIXEL_TO_METERS(jumpStartPos.getY() + texH / 2);
            pbody->body->SetTransform(correctedPos, 0);
        }

        // Create attack area on landing
        if (!jumpAttackArea) {
            UpdateJumpAttackArea();
        }

        // End jump attack
        jumpAttackActive = false;
        isJumping = false;
        isLanding = false;
        shadowVisible = false;
        hasReachedPeak = false;
        startFalling = false;

        // Reset physics and animation
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        pbody->body->SetGravityScale(1.0f);
        currentAttackCooldown = attackCooldown;
        currentAnimation = &idle2;
        currentAnimation->Reset();

        // Clean up attack area
        if (jumpAttackArea) {
            Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
            jumpAttackArea = nullptr;
        }
    }
}

void Devil::UpdateJumpAttackArea() {
    Vector2D currentPos = GetPosition();

    jumpAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        currentPos.getX(),
        currentPos.getY(),
        texW + 50, texH + 50, // Larger attack area for jump attack
        bodyType::KINEMATIC
    );

    jumpAttackArea->listener = this;
    jumpAttackArea->ctype = ColliderType::DEVIL_JUMP_ATTACK2;
    jumpAttackArea->body->SetFixedRotation(true);
}

void Devil::RenderShadow() {
    if (shadowVisible && shadowTexture) {
        int groundY = initY + texH; // Simple ground calculation

        Engine::GetInstance().render.get()->DrawTexture(
            shadowTexture,
            (int)shadowPosition.getX() - 32,
            groundY + 50,
            nullptr,
            1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE
        );
    }
}

void Devil::ResetPath() {
    if (pathfinding) {
        Vector2D pos = GetPosition();
        Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
        pathfinding->ResetPath(tilePos);
    }
}

void Devil::HandleTransformation(float dt) {
    static int transformStep = 0;
    static float transformTimer = 0.0f;

    switch (transformStep) {
    case 0:
        // Play defeat animation
        currentAnimation = &defeat;
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        currentAnimation->Update();

        if (currentAnimation->HasFinished()) {
            defeat.Reset();
            transformStep = 1;
        }
        break;

    case 1:
        // Play transformation animation
        if (currentAnimation != &transform) {
            currentAnimation = &transform;
            transform.Reset();
            transformTimer = 0.0f;

            if (currentPhase == 1) {
                ResizeCollisionForPhase2();
            }
        }

        currentAnimation->Update();
        transformTimer += dt;

        if (currentAnimation->HasFinished()) {
            transformStep = 2;
        }
        break;

    case 2:
        // Complete transformation
        currentPhase++;
        isTransforming = false;

        if (currentPhase == 2) {
            currentAnimation = &idle2;
            Engine::GetInstance().ui->fase1 = false; // Hide Phase 1 UI
            Engine::GetInstance().ui->fase2 = true;  // Show Phase 2 UI
        }
        else if (currentPhase == 3) {
            currentAnimation = &idle3;
            Engine::GetInstance().ui->fase2 = false; // Hide Phase 2 UI
            Engine::GetInstance().ui->fase3 = true;  // Show Phase 3 UI
        }

        transformStep = 0;
        transformTimer = 0.0f;
        Hiteado = false;
        break;
    }
}

void Devil::ResizeCollisionForPhase2() {
    if (pbody) {
        b2Vec2 currentPos = pbody->body->GetTransform().p;

        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);

        int newWidth = texW + 180;
        int newHeight = texH + 180;

        pbody = Engine::GetInstance().physics.get()->CreateCircle(
            METERS_TO_PIXELS(currentPos.x),
            METERS_TO_PIXELS(currentPos.y),
            newWidth / 2 - 3,
            bodyType::DYNAMIC
        );

        pbody->ctype = ColliderType::DEVIL;
        pbody->body->SetGravityScale(1.0f);
        pbody->listener = this;
    }
}

void Devil::UpdatePunchAttackArea() {
    if (punchAttackArea) {
        int punchX = position.getX() + texW / 2;
        int punchY = position.getY() + texH / 2;
        punchX += isLookingLeft ? -10 : 10;
        punchAttackArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(punchX), PIXEL_TO_METERS(punchY)), 0);
    }
}

void Devil::CreatePunchAttack() {
    isAttacking = true;
    currentAnimation = &punch;
    currentAnimation->Reset();

    if (!punchAttackArea) {
        int punchX = position.getX() + texW / 2;
        int punchY = position.getY() + texH / 2;
        punchX += isLookingLeft ? -10 : 10;

        punchAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            punchX, punchY,
            texW + 20, texH / 2,
            bodyType::KINEMATIC
        );

        punchAttackArea->listener = this;
        punchAttackArea->ctype = ColliderType::DEVIL_PUNCH_ATTACK1;
        punchAttackArea->body->SetFixedRotation(true);
    }
}

void Devil::UpdatePosition() {
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
}

void Devil::RenderSprite() {
    SDL_Rect frame = currentAnimation->GetCurrentFrame();
    int offsetX = 0;
    int offsetY = 0;

    flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // Adjust sprite offsets based on animation and phase
    if (currentAnimation == &transform || currentPhase >= 2) {
        offsetY = -137;
        if (isLookingLeft) offsetX = -130;
        if (!isLookingLeft) offsetX = -100;
    }

    if (currentAnimation == &idle2) {
        offsetY = -137;
    }

    if (currentAnimation == &salto || currentAnimation == &land) {
        offsetY = -425;
    }

    if (currentAnimation == &transform2 || currentAnimation == &idle3 || currentAnimation == &attack) {
        offsetY = -325;
    }

    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX() + offsetX,
        (int)position.getY() + offsetY,
        &frame,
        1.0f, 0.0, INT_MAX, INT_MAX, flip
    );
}

void Devil::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLAYER:
    case ColliderType::PLATFORM:
        break;

    case ColliderType::PLAYER_ATTACK:
        if (!Hiteado && !isTransforming) {
            Hiteado = true;
            lives--;
            live1--;

            if (live1 == 0) {
                isTransforming = true;

                // Cancel current attack if active
                if (isAttacking) {
                    isAttacking = false;
                    if (punchAttackArea) {
                        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                        punchAttackArea = nullptr;
                    }
                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }
            }
            else {
                isDying = true;
            }
        }
        break;

    case ColliderType::PLAYER_WHIP_ATTACK:
        if (!Hiteado && !isTransforming) {
            Hiteado = true;
            lives--;

            if (lives > 0) {
                isTransforming = true;

                // Cancel current attack if active
                if (isAttacking) {
                    isAttacking = false;
                    if (punchAttackArea) {
                        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                        punchAttackArea = nullptr;
                    }
                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }

                // Cancel jump attack if active
                if (jumpAttackActive) {
                    jumpAttackActive = false;
                    isJumping = false;
                    isLanding = false;
                    shadowVisible = false;
                    if (jumpAttackArea) {
                        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
                        jumpAttackArea = nullptr;
                    }
                    pbody->body->SetGravityScale(1.0f);
                }
            }
            else {
                isDying = true;
            }
        }
        break;
    }
}

void Devil::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
}

bool Devil::CleanUp() {
    if (punchAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
        punchAttackArea = nullptr;
    }
    if (jumpAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
        jumpAttackArea = nullptr;
    }
    if (pathfinding) {
        delete pathfinding;
        pathfinding = nullptr;
    }
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void Devil::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Devil::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    return Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
}

void Devil::ResetLives() {
    lives = 3;
    currentPhase = 1;
    currentAnimation = &idle;
    isDying = false;
    isTransforming = false;
    jumpAttackActive = false;
    isJumping = false;
    isLanding = false;
    shadowVisible = false;
}