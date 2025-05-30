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
    live1 = 1;//Live for phase 1
    live2;//Live for phase 2
    live3;//live for phase 3
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

    idle.LoadAnimations(parameters.child("animations").child("idle"));
    walk.LoadAnimations(parameters.child("animations").child("walk"));
    punch.LoadAnimations(parameters.child("animations").child("punch"));
    defeat.LoadAnimations(parameters.child("animations").child("defeat"));
    transform.LoadAnimations(parameters.child("animations").child("transform"));
    idle2.LoadAnimations(parameters.child("animations").child("idle2"));
    salto.LoadAnimations(parameters.child("animations").child("salto"));
    land.LoadAnimations(parameters.child("animations").child("land"));
    currentAnimation = &idle;

    moveSpeed = 2.0f;
    patrolSpeed = 3.0f;

    pbody = Engine::GetInstance().physics.get()->CreateCircle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW / 2 - 3,
        bodyType::DYNAMIC
    );

    pbody->ctype = ColliderType::DEVIL;
    pbody->body->SetGravityScale(1.0f);

    pathfinding = new Pathfinding();

    return true;
}

bool Devil::Update(float dt) {
    bool isGameplay = Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY;

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

    if (isTransforming) {
        HandleTransformation(dt);
        UpdatePosition();
        RenderSprite();
        return true;
    }

    // Handle jump attack if active
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

    enemyPos = GetPosition();
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    float dx = playerTilePos.getX() - enemyTilePos.getX();
    float distanceToPlayer = abs(dx);
    isLookingLeft = dx < 0;

    // Phase control: 1 = Active, 2 = Static, 3 = Placeholder (future behavior)
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

    // Debug: draw enemy path
    if (isChasing && !pathfinding->pathTiles.empty()) {
        pathfinding->DrawPath();
    }

    if (Engine::GetInstance().ui->figth3 == true) {
        //UI Lives
        if(currentPhase == 1){ Engine::GetInstance().ui->vidab3 = live1; }
        if (currentPhase == 2) { Engine::GetInstance().ui->vidab3 = lives; /* live2; */ }
        if (currentPhase == 3) { Engine::GetInstance().ui->vidab3 = lives; /*live3;*/ }
        
    }
    return true;
}

void Devil::HandlePhase1(float distanceToPlayer, float dx, float dt) {
    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    const float DETECTION_DISTANCE = 12.0f;
    const float CHASE_SPEED = moveSpeed;
    const int MAX_PATHFINDING_ITERATIONS = 50;

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
        if (!canAttack) {
            currentAttackCooldown -= dt;
            if (currentAttackCooldown <= 0) {
                canAttack = true;
                currentAttackCooldown = 0.0f;
            }
        }

        if (distanceToPlayer <= 3.0f && canAttack) {
            CreatePunchAttack();
        }
        else if (distanceToPlayer <= DETECTION_DISTANCE) {
            isChasing = true;

            Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
            Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
            Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

            ResetPath();

            for (int i = 0; i < MAX_PATHFINDING_ITERATIONS; i++) {
                pathfinding->PropagateAStar(SQUARED);
                if (pathfinding->ReachedPlayer(playerTilePos)) {
                    break;
                }
            }

            pathfinding->ComputePath(playerTilePos.getX(), playerTilePos.getY());
            currentAnimation = &walk;

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
        else {
            isChasing = false;
            currentAnimation = &idle;
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        }
    }
}

void Devil::HandlePhase2(float distanceToPlayer, float dx, float dt) {
    const float JUMP_ATTACK_DISTANCE = 8.0f;

    if (!canAttack) {
        currentAttackCooldown -= dt;
        if (currentAttackCooldown <= 0) {
            canAttack = true;
            currentAttackCooldown = 0.0f;
        }
    }

    // Jump attack logic for Phase 2
    if (distanceToPlayer <= JUMP_ATTACK_DISTANCE && canAttack && !jumpAttackActive) {
        CreateJumpAttack();
    }
    else if (!jumpAttackActive) {
        currentAnimation = &idle2;
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
    }
}

void Devil::HandlePhase3(float distanceToPlayer, float dx, float dt) {
    currentAnimation = &idle2;
    pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
}

void Devil::CreateJumpAttack() {
    jumpAttackActive = true;
    isJumping = true;
    isLanding = false;
    canAttack = false;
    
    // Get player position for targeting
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    targetLandingPos = playerPos;
    
    // Set shadow position to target landing position
    shadowPosition = targetLandingPos;
    shadowVisible = true;
    
    // Start jump animation
    currentAnimation = &salto;
    currentAnimation->Reset();
    
    // Store starting position
    jumpStartY = enemyPos.getY();
    
    // Apply upward force for jump
    pbody->body->SetLinearVelocity(b2Vec2(0, -jumpSpeed));
    pbody->body->SetGravityScale(0.5f); // Reduce gravity during jump
    
    LOG("Devil Phase 2: Jump attack started!");
}

void Devil::UpdateJumpAttack(float dt) {
    Vector2D currentPos = GetPosition();
    
    // Always update shadow position to follow boss X position during jump attack
    if (shadowVisible) {
        shadowPosition.setX(currentPos.getX());
        // Keep the same Y position (ground level) for shadow
    }
    
    if (isJumping) {
        // Check if we've reached peak height or jump animation finished
        if (currentPos.getY() <= jumpStartY - jumpHeight || currentAnimation->HasFinished()) {
            isJumping = false;
            isLanding = true;
            
            // Start landing animation
            currentAnimation = &land;
            currentAnimation->Reset();
            
            // Calculate horizontal movement towards target
            float horizontalDistance = targetLandingPos.getX() - currentPos.getX();
            float landingVelocityX = horizontalDistance * 0.05f; // Adjust speed as needed
            
            // Apply landing velocity
            pbody->body->SetLinearVelocity(b2Vec2(landingVelocityX, jumpSpeed * 0.5f));
            pbody->body->SetGravityScale(2.0f); // Increase gravity for faster fall
            
            LOG("Devil Phase 2: Starting to land!");
        }
    }
    else if (isLanding) {
        // Check if we've landed (close to ground or landing animation finished)
        if (currentAnimation->HasFinished()) {
            // Create attack area at landing position
            if (!jumpAttackArea) {
                UpdateJumpAttackArea();
            }
            
            // End jump attack
            jumpAttackActive = false;
            isLanding = false;
            shadowVisible = false;
            
            // Reset physics
            pbody->body->SetGravityScale(1.0f);
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
            
            // Set cooldown
            currentAttackCooldown = attackCooldown;
            currentAnimation = &idle2;
           
            if (jumpAttackArea) {
                Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
                jumpAttackArea = nullptr;
            }
            
            LOG("Devil Phase 2: Jump attack completed!");
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
        int groundY = initY + texH; // Simple ground calculation (falta ajustarlo)
        
        Engine::GetInstance().render.get()->DrawTexture(
            shadowTexture,
            (int)shadowPosition.getX() - 32, 
            groundY - 16,
            nullptr, // Full texture
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
        currentAnimation = &defeat;
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        currentAnimation->Update();

        if (currentAnimation->HasFinished()) {
            defeat.Reset();
            transformStep = 1;
            LOG("Defeat animation finished, starting transform");
        }
        break;

    case 1:
        if (currentAnimation != &transform) {
            currentAnimation = &transform;
            transform.Reset();
            transformTimer = 0.0f;

            if (currentPhase == 1) {
                ResizeCollisionForPhase2();
                LOG("Transform started - collision resized for Phase 2");
            }
        }

        currentAnimation->Update();
        transformTimer += dt;

        if (currentAnimation->HasFinished()) {
            transformStep = 2;
            LOG("Transform animation finished");
        }
        break;

    case 2:
        currentPhase++;
        isTransforming = false;

        if (currentPhase == 2) {
            currentAnimation = &idle2;
            Engine::GetInstance().ui->fase1 = false;//Quit live boss UI from Phase 1 from the screen 
            Engine::GetInstance().ui->fase2 = true;//Put live boss UI from Phase 2 in screen
            LOG("Devil entered Phase 2!");
        }
        else if (currentPhase == 3) {
            currentAnimation = &idle2;
            Engine::GetInstance().ui->fase2 = false;//Quit live bar boss UI from Phase 2 from the screen 
            Engine::GetInstance().ui->fase3 = true;//Put live bar boss UI from Phase 3 in screen
            LOG("Devil entered Phase 3!");
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

        LOG("Devil collision body resized for Phase 2");
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

    if (currentAnimation == &transform || currentPhase >= 2) {
        offsetY = -137;
        if (isLookingLeft) offsetX = -130;
        if (!isLookingLeft) offsetX = -100;
    }

    if (currentAnimation == &idle2 ) {
        if (isLookingLeft) offsetY = -137;
        if (!isLookingLeft) offsetY = -137;
    }
    if ( currentAnimation == &salto || currentAnimation == &land) {
        if (isLookingLeft) offsetY = -425;
        if (!isLookingLeft) offsetY = -425;
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
            LOG("Devil hit! Lives remaining: %d, Current Phase: %d", lives, currentPhase);

            if (live1 == 0) {
                isTransforming = true;

                if (isAttacking) {
                    isAttacking = false;
                    if (punchAttackArea) {
                        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                        punchAttackArea = nullptr;
                    }
                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }

                LOG("Devil starting transformation to Phase %d!", currentPhase + 1);
            }
            else {
                LOG("Devil defeated!");
                isDying = true;
            }
        }
        break;
    case ColliderType::PLAYER_WHIP_ATTACK:
        if (!Hiteado && !isTransforming) {
            Hiteado = true;
            lives--;
            LOG("Devil hit! Lives remaining: %d, Current Phase: %d", lives, currentPhase);

            if (lives > 0) {
                isTransforming = true;

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

                LOG("Devil starting transformation to Phase %d!", currentPhase + 1);
            }
            else {
                LOG("Devil defeated!");
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