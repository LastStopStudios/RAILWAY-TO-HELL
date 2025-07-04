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
#include "GlobalSettings.h"
#include "Spears.h"
#include "Ffmpeg.h"

Devil::Devil() : Entity(EntityType::DEVIL)
{
    currentPhase = 1;
    lives = 2; // 3 lives for 3 phases
    live1 = 3;//Live for phase 1
    live2 = 6;//Live for phase 2
    live3 = 8;//live for phase 3
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
    // Load textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    shadowTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/bosses/Shadow.png");

    // Set initial position and dimensions
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    initX = parameters.attribute("x").as_int();
    initY = parameters.attribute("y").as_int();

    // Load all animations from XML
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
    attackH.LoadAnimations(parameters.child("animations").child("attackH"));
    attackV.LoadAnimations(parameters.child("animations").child("attackV"));
    death.LoadAnimations(parameters.child("animations").child("death"));
    currentAnimation = &idle;

    moveSpeed = 2.0f;
    patrolSpeed = 3.0f;

    // Create physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW / 2 - 3,
        bodyType::DYNAMIC
    );

    pbody->ctype = ColliderType::DEVIL;
    pbody->body->SetGravityScale(10.0f);

    pugi::xml_document configDoc;
    if (!configDoc.load_file("config.xml")) {
        LOG("ERROR: Could not load config.xml");
        return false;
    }

    

    if (!spearConfigDoc.load_file("config.xml")) {
        LOG("ERROR: Could not load config.xml");
        return false;
    }

    pugi::xml_node spearTemplateNode = spearConfigDoc.child("config").child("scene12").child("entities").child("spear");

    this->spearTemplateNode = spearTemplateNode; 

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
        if (pbody && pbody->body && !isTransforming) {
            pbody->body->SetGravityScale(1.0f);
        }
    }
    bool bossFightActive = Engine::GetInstance().dialogoM->bossFightReady;
    if(bossFightActive){
        // Update UI health display
        if (Engine::GetInstance().ui->figth3 == true) {
            //UI Lives
            if (currentPhase == 1) { Engine::GetInstance().ui->vidab3 = live1;/*UI lives boss phase 1 */ }
            if (currentPhase == 2) { Engine::GetInstance().ui->vidab3 = live2; /*UI lives boss phase 2 */ }
            if (currentPhase == 3) { Engine::GetInstance().ui->vidab3 = live3; /*UI lives boss phase 3 */ }
        }

        //Handle Dead
        if (isDying) {
            // Ensure that there is no movement during death
            if (pbody != nullptr) {
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
                pbody->body->SetGravityScale(1.0f);
            }

            currentAnimation->Update();

            if (currentAnimation->HasFinished()) {
                Engine::GetInstance().scene->morido = true;
            }
            // Draw the death animation
            SDL_Rect frame = currentAnimation->GetCurrentFrame();
            RenderSprite();
            //UI Lives
            Engine::GetInstance().ui->figth3 = false;



            return true;
        }

        // Handle transformation state
        if (isTransforming) {
            HandleTransformation(dt);
            UpdatePosition();
            RenderSprite();
            return true; 
        }

        // Handle jump attack mechanics
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

        // Get player and enemy positions
        enemyPos = GetPosition();
        Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
        Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
        Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

        float dx = playerTilePos.getX() - enemyTilePos.getX();
        float distanceToPlayer = abs(dx);
        isLookingLeft = dx < 0;

        // Phase-based 
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
            break;
        }

        UpdatePosition();
        RenderSprite();
        if (shadowVisible) {
            RenderShadow();
        }
        currentAnimation->Update();

        // Debug pathfinding
        if (isChasing && !pathfinding->pathTiles.empty()) {
            pathfinding->DrawPath();
        }
    }
    UpdatePosition();
    RenderSprite();
    currentAnimation->Update();
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

        // Close range punch attack
        if (distanceToPlayer <= 3.0f && canAttack) {
            CreatePunchAttack();
        }
        // Chase using pathfinding
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
        // Idle when player is far
        else {
            isChasing = false;
            currentAnimation = &idle;
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        }
    }
}

void Devil::HandlePhase2(float distanceToPlayer, float dx, float dt) {
    const float TAIL_ATTACK_DISTANCE = 6.5f;
    const float JUMP_ATTACK_DISTANCE = 100.0f;

    // Update attack cooldown
    if (!canAttack) {
        currentAttackCooldown -= dt;
        if (currentAttackCooldown <= 0) {
            canAttack = true;
            currentAttackCooldown = 0.0f;
        }
    }

    // Handle tail attack animation
    if (isTailAttacking && currentAnimation == &colatazo) {
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));

        UpdateTailAttackArea();

        if (currentAnimation->HasFinished()) {
            isTailAttacking = false;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation = &idle2;

            if (tailAttackArea) {
                Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                tailAttackArea = nullptr;
            }
            colatazo.Reset();
        }
    }
    // Create new attacks only if not already attacking
    else if (!isTailAttacking && !jumpAttackActive) {
        // Close range tail attack
        if (distanceToPlayer <= TAIL_ATTACK_DISTANCE && canAttack) { 
          CreateTailAttack();
        }
        // Medium range jump attack
        else if (distanceToPlayer <= JUMP_ATTACK_DISTANCE && distanceToPlayer > TAIL_ATTACK_DISTANCE && canAttack) {
            int x, y;
            pbody->GetPosition(x, y);
            CreateJumpAttack();
        }
        // Idle state
        else {
            currentAnimation = &idle2;
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));

        }
    }
}

void Devil::HandlePhase3(float distanceToPlayer, float dx, float dt) {
    // Update cooldown if it's not attacking or waiting to throw spears
    if (!canAttack && !isSpearAttacking && !waitingToLaunchSpears) {
        currentSpearCooldown -= dt;
        if (currentSpearCooldown <= 0) {
            canAttack = true;
            currentSpearCooldown = 0.0f;
        }
    }

    // Waiting for the animation to finish in order to throw spears
    if (waitingToLaunchSpears) {
        currentAnimation->Update();

        if (currentAnimation->HasFinished()) {
            if (isVerticalSpearAttack) {
                CreateVerticalSpearAttack();  // This is where spears really take off.
            }
            else if (isHorizontalSpearAttack) {
                CreateHorizontalSpearAttack();
            }

            waitingToLaunchSpears = false;
            isSpearAttacking = true;
        }

        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        return;
    }

    // Updating active spears
    if (isSpearAttacking) {
        UpdateSpearAttacks(dt);
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        return;
    }

    // Choose attack
    if (canAttack && !waitingToLaunchSpears) {
        srand(time(NULL));
        int attackChoice = rand() % 2;

        if (attackChoice == 0) {
            currentAnimation = &attackV;
            isVerticalSpearAttack = true;
            isHorizontalSpearAttack = false;
        }
        else {
            currentAnimation = &attackH;
            isVerticalSpearAttack = false;
            isHorizontalSpearAttack = true;
        }

        currentAnimation->Reset();
        waitingToLaunchSpears = true;
        canAttack = false;
        currentSpearCooldown = spearAttackCooldown;

        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
        return;
    }

    // Idle 
    if (currentAnimation != &idle3) {
        currentAnimation = &idle3;
        currentAnimation->Reset();
    }
    pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));
}
void Devil::CreateVerticalSpearAttack() {
    isSpearAttacking = true;
    isVerticalSpearAttack = true;
    isHorizontalSpearAttack = false;
    canAttack = false;

    currentAnimation = &attackV;
    currentAnimation->Reset();

    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    int numSpears = 4 + (rand() % 2); // Spawn between 3-4 spears (before there where 5-7)

    for (int i = 0; i < numSpears; i++) {
        Spears* spear = (Spears*)Engine::GetInstance().entityManager.get()->CreateEntity(EntityType::SPEAR);

        if (spear) {
            spear->SetParameters(spearTemplateNode);
            spear->SetDirection(SpearDirection::VERTICAL_DOWN);

            // Greater separation between spears(more scattered)
            float offsetX = (i - numSpears / 2) * 200.0f + ((rand() % 60) - 30); 
            float spearX = playerPos.getX() + offsetX;
            float spearY = playerPos.getY() - 2000;
            spear->SetOriginPosition(Vector2D(spearX, spearY));

            if (spear->Awake() && spear->Start()) {
                activeSpears.push_back(spear);
            }
            else {
                spear->pendingToDelete = true;
            }
        }
    }
}

void Devil::CreateHorizontalSpearAttack() {
    isSpearAttacking = true;
    isVerticalSpearAttack = false;
    isHorizontalSpearAttack = true;
    canAttack = false;

    currentAnimation = &attackH;
    currentAnimation->Reset();

    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D currentPos = GetPosition();

    // Now 3 horizontal spears(one for each height)
    int numSpears = 3;

	// Heights for the spears 
    float heights[3] = {
		currentPos.getY() - 250,  // High height of the Devil
		currentPos.getY() - 40,  // Mid height of the Devil
		currentPos.getY() + 30   // Low height of the Devil
    };

    for (int i = 0; i < numSpears; i++) {
        Spears* spear = (Spears*)Engine::GetInstance().entityManager.get()->CreateEntity(EntityType::SPEAR);

        if (spear) {
            spear->SetParameters(spearTemplateNode);

            // Use predefined heights
            float spearY = heights[i];
            float spearX;
            SpearDirection direction;

            // Random firection, either left or right
            bool fromLeft = (rand() % 2) == 0; // 50% change each site

            if (fromLeft) {
				// Left side launch
                spearX = currentPos.getX() - 1100;
                direction = SpearDirection::HORIZONTAL_RIGHT;
            }
            else {
				// Right side launch
                spearX = currentPos.getX() + 600;
                direction = SpearDirection::HORIZONTAL_LEFT;
            }

            spear->SetDirection(direction);
            spear->SetOriginPosition(Vector2D(spearX, spearY));

            if (spear->Awake() && spear->Start()) {
                activeSpears.push_back(spear);
            }
            else {
                spear->pendingToDelete = true;
            }
        }
    }
}
void Devil::UpdateSpearAttacks(float dt) {
    currentAnimation->Update();

    for (auto it = activeSpears.begin(); it != activeSpears.end();) {
        Spears* spear = *it;

        if (spear == nullptr || spear->pendingToDelete) {
            it = activeSpears.erase(it);
            continue;
        }

        Vector2D spearPos = spear->GetPosition();
        bool shouldDelete = false;

        // Out-of-bounds check depending on attack type
        if (isVerticalSpearAttack && spearPos.getY() > initY + 400) {
            shouldDelete = true;
        }
        else if (isHorizontalSpearAttack &&
            (spearPos.getX() < -300 || spearPos.getX() > 2200)) {
            shouldDelete = true;
        }

        if (spear->isDisappearing) {
            shouldDelete = true;
        }

        if (shouldDelete) {
            spear->pendingToDelete = true;
            it = activeSpears.erase(it);
            continue;
        }

        ++it;
    }

    static float attackTimeout = 0.0f;
    attackTimeout += dt;

    bool animationFinished = currentAnimation->HasFinished();
    bool allSpearsGone = activeSpears.empty();

    // End attack if animation is done and spears are gone, or after timeout
    if ((animationFinished && allSpearsGone) || attackTimeout > 5.0f) {
        isSpearAttacking = false;
        isVerticalSpearAttack = false;
        isHorizontalSpearAttack = false;
        currentSpearCooldown = spearAttackCooldown;
        attackTimeout = 0.0f;

        currentAnimation = &idle3;
        currentAnimation->Reset();
    }
}

void Devil::CleanupSpears() {
    activeSpears.clear();
}

void Devil::CreateJumpAttack() {
    jumpAttackActive = true;
    jumpPreparation = true;
    isJumping = false;
    isLanding = false;
    landingComplete = false;
    fallAnimationLocked = false;
    canAttack = false;
    hasReachedPeak = false;
    startFalling = false;
    hasReachedMaxHeight = false;
    currentJumpHeight = 0.0f;

    Vector2D currentPos = GetPosition();
    jumpStartPos = currentPos;

    
    if (pendingTransformation) {
        targetLandingPos = Vector2D(1200, currentPos.getY());
        targetPlayerX = 1315;
    }
    else {
		// Jump to player's position
        Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
        targetLandingPos = playerPos;
        targetPlayerX = playerPos.getX();
    }

    // Start with jump animation for preparation
    currentAnimation = &salto;
    currentAnimation->Reset();

    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
    pbody->body->SetGravityScale(1.0f);
}

void Devil::UpdateJumpAttack(float dt) {
    Vector2D currentPos = GetPosition();
    b2Vec2 currentVelocity = pbody->body->GetLinearVelocity();

	//Check if the boss is failling out of the map
    if (currentPos.getY() > 1706) {
        Vector2D safePosition = Vector2D(currentPos.getX(), 1619);
        SetPosition(safePosition);

        b2Vec2 correctedPos = b2Vec2(PIXEL_TO_METERS(safePosition.getX() + texW / 2), PIXEL_TO_METERS(safePosition.getY() + texH / 2));
        pbody->body->SetTransform(correctedPos, 0);
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));

        landingComplete = true;
        isJumping = false;
        startFalling = false;
        currentPos = safePosition;  
    }

    // Update shadow position
    if (shadowVisible) {
        shadowPosition.setX(currentPos.getX() - 80);
    }

    // Jump preparation phase (first 6 frames)
    if (jumpPreparation) {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
        currentAnimation->Update();

        if (currentAnimation->GetCurrentFrameIndex() >= 6) {
            jumpPreparation = false;
            isJumping = true;

            // show the shadow when preparation ends
            shadowPosition = targetLandingPos;
            shadowVisible = true;

            pbody->body->SetLinearVelocity(b2Vec2(0.0f, -40.0f));
            pbody->body->SetGravityScale(1.0f);
        }
        return;
    }

    if (landingComplete) {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        currentAnimation->Update();

        if (currentAnimation->HasFinished()) {
            jumpAttackActive = false;
            landingComplete = false;
            // shadowVisible = false;  
            fallAnimationLocked = false;
            jumpAnimationLocked = false;
            hasReachedMaxHeight = false;
            currentJumpHeight = 0.0f;

            pbody->body->SetGravityScale(1.0f);

			// Activate pending transformation
            if (pendingTransformation) {
                isTransforming = true;
                pendingTransformation = false;
            }
            else {
                currentAttackCooldown = attackCooldown;
                currentAnimation = &idle2;
                currentAnimation->Reset();
            }

            if (jumpAttackArea) {
                Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
                jumpAttackArea = nullptr;
            }
        }
        Vector2D NcurrentPos = GetPosition();
        if (NcurrentPos.getY() > 1760) {
            position.setY(1619);
        }
        return;
    }

    // Calculate current jump height
    if (isJumping) {
        currentJumpHeight = jumpStartPos.getY() - currentPos.getY();

        // Check if reached maximum height
        if (currentJumpHeight >= maxJumpHeight && !hasReachedMaxHeight) {
            hasReachedMaxHeight = true;
            hasReachedPeak = true;
            pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

            // Lock animation at frame 8
            if (currentAnimation == &salto) {
                currentAnimation->currentFrame = 8.0f;
                jumpAnimationLocked = true;
            }

        }
    }

    // Check if reached peak of jump
    if (!hasReachedPeak && (currentVelocity.y >= 0 || hasReachedMaxHeight) && isJumping) {
        hasReachedPeak = true;
        Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
        targetPlayerX = playerPos.getX();

        // Lock animation at frame 8
        if (currentAnimation == &salto) {
            currentAnimation->currentFrame = 8.0f;
            jumpAnimationLocked = true;
        }
    }

    // Update animation during upward movement only
    if (isJumping && !hasReachedPeak && !jumpAnimationLocked && !hasReachedMaxHeight) {
        currentAnimation->Update();
    }
    else if (startFalling && fallAnimationLocked && currentAnimation == &land) {
        currentAnimation->currentFrame = 0.0f;
    }

    // Force frame 8 during horizontal movement
    if (hasReachedPeak && !startFalling && currentAnimation == &salto) {
        currentAnimation->currentFrame = 8.0f;
    }

    // Horizontal movement at peak
    if (hasReachedPeak && !startFalling) {
        float distanceToPlayerX = abs(currentPos.getX() - targetPlayerX);

        // Keep animation on last frame
        if (currentAnimation == &salto) {
            currentAnimation->currentFrame = currentAnimation->totalFrames - 1.0f;
        }

        bool shouldStartFalling = (distanceToPlayerX < 20.0f) || (currentVelocity.y > 0.5f);

        if (shouldStartFalling) {
            // Start falling dive
            startFalling = true;
            isLanding = true;
            jumpAnimationLocked = false;
            pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            pbody->body->SetGravityScale(5.0f);

            // Switch to land animation
            currentAnimation = &land;
            currentAnimation->Reset();
            currentAnimation->currentFrame = 0.0f;
            fallAnimationLocked = true;
        }
        else {
            // Continue horizontal movement
            float horizontalDirection = (targetPlayerX > currentPos.getX()) ? 1.0f : -1.0f;
            pbody->body->SetLinearVelocity(b2Vec2(horizontalDirection * 8.0f, 0.0f));
            pbody->body->SetGravityScale(0.0f);

            // Lock animation frame
            if (currentAnimation == &salto) {
                currentAnimation->currentFrame = 8.0f;
                float originalSpeed = currentAnimation->speed;
                currentAnimation->speed = 0.0f;
                currentAnimation->speed = originalSpeed;
            }
        }
    }

    // Vertical dive during fall
    if (startFalling && !landingComplete) {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, 30.0f));
        if (currentAnimation == &land) {
            currentAnimation->currentFrame = 0.0f;
        }
    }

    // Check for landing
    if (hasReachedPeak && currentPos.getY() >= jumpStartPos.getY() - 20) {
        // Position correction if needed
        if (currentPos.getY() > jumpStartPos.getY()) {
            b2Vec2 correctedPos = pbody->body->GetPosition();
            correctedPos.y = PIXEL_TO_METERS(jumpStartPos.getY() + texH / 2);
            pbody->body->SetTransform(correctedPos, 0);
        }

        // Create attack area
        if (!jumpAttackArea) {
            UpdateJumpAttackArea();
        }

        // DESTROY SHADOW IMMEDIATELY WHEN HITTING GROUND - BEFORE RECOVERY ANIMATION
        if (shadowVisible) {
            shadowVisible = false;
        }

        // Start landing completion
        if (!landingComplete) {
            landingComplete = true;
            isJumping = false;
            isLanding = false;
            hasReachedPeak = false;
            startFalling = false;
            fallAnimationLocked = false;
            jumpAnimationLocked = false;
            hasReachedMaxHeight = false;
            currentJumpHeight = 0.0f;

            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(1.0f);

            currentAnimation = &land;
            currentAnimation->Reset();
            currentAnimation->currentFrame = 0.0f;
        }
    }
}

void Devil::CreateTailAttack() {
    if (isTailAttacking) {
        return;
    }

    isTailAttacking = true;
    currentAnimation = &colatazo;
    currentAnimation->Reset();
    canAttack = false;

    // Clean up previous attack area
    if (tailAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
        tailAttackArea = nullptr;
    }

    // Create tail attack area
    int tailX = position.getX() + texW / 2;
    int tailY = position.getY() + 140;

    // Adjust position based on direction
    tailX += isLookingLeft ? -90 : 90;

    tailAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        tailX, tailY,
        texW + 120, 20,
        bodyType::KINEMATIC
    );

    tailAttackArea->listener = this;
    tailAttackArea->ctype = ColliderType::DEVIL_TAIL_ATTACK;
    tailAttackArea->body->SetFixedRotation(true);
}

void Devil::UpdateTailAttackArea() {

    if (tailAttackArea && isTailAttacking){
         
        int tailX = position.getX() + texW / 2;
        int tailY = position.getY()  + 140;
        tailX += isLookingLeft ? -90 : 90;
        tailAttackArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(tailX), PIXEL_TO_METERS(tailY)), 0);
    }
}

void Devil::UpdateJumpAttackArea() {
    Vector2D currentPos = GetPosition();

    jumpAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        currentPos.getX(),
        currentPos.getY() + 85,
        texW + 100, texH ,
        bodyType::KINEMATIC
    );

    jumpAttackArea->listener = this;
    jumpAttackArea->ctype = ColliderType::DEVIL_JUMP_ATTACK2;
    jumpAttackArea->body->SetFixedRotation(true);
}

void Devil::RenderShadow() {
    if (shadowVisible && shadowTexture) {
        int groundY = initY + texH;

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
	// Stop boss while transforming
    if (pbody && pbody->body) {
        pbody->body->SetLinearVelocity(b2Vec2(0, 0));
        pbody->body->SetAngularVelocity(0);
        pbody->body->SetGravityScale(0.0f);

        if (pbody->body->GetType() != b2_kinematicBody) {
            pbody->body->SetType(b2_kinematicBody);
        }
    }

    switch (transformStep) {
    case 0:
        if (currentPhase == 1) {
            if (currentAnimation != &defeat) {
                currentAnimation = &defeat;
                defeat.Reset();
            }

            currentAnimation->Update();

            if (currentAnimation->HasFinished()) {
                defeat.Reset();
                transformStep = 1;
            }
        }
        else if (currentPhase == 2) {
			// Do zoom effect for Phase 2->3 transformation

            if (!zooming) {
                zooming = true;
                zoom = GlobalSettings::GetInstance().GetTextureMultiplier();
            }

            if (zooming) {
                zoom -= 0.01f;
                if (zoom <= 1.0f) {
                    zoom = 1.0f;
                    zooming = false;
                    transformStep = 1;
                }
                GlobalSettings::GetInstance().SetTextureMultiplier(zoom);
            }
        }
        break;

    case 1:

        if (currentPhase == 1) {
            // Phase 1 -> Phase 2: use transform animation
            if (currentAnimation != &transform) {
                currentAnimation = &transform;
                transform.Reset();
                transformTimer = 0.0f;
                ResizeCollisionForPhase2();
            }
            currentAnimation->Update();
        }
        else if (currentPhase == 2) {
            // Phase 2 -> Phase 3: use transform2 animation
            if (currentAnimation != &transform2) {
                currentAnimation = &transform2;
                transform2.Reset();
                transformTimer = 0.0f;
                ResizeCollisionForPhase3();
            }
            currentAnimation->Update();
        }

        transformTimer += dt;

        if (currentAnimation->HasFinished()) {
            transformStep = 2;
        }
        break;

    case 2:

        // Complete transformation
        currentPhase++;
        isTransforming = false;

        if (pbody && pbody->body) {
            pbody->body->SetType(b2_dynamicBody);
            pbody->body->SetGravityScale(1.0f); 
        }

        if (currentPhase == 2) {
            currentAnimation = &idle2;
            Engine::GetInstance().ui->fase1 = false;
            Engine::GetInstance().ui->fase2 = true;
        }
        else if (currentPhase == 3) {
            currentAnimation = &idle3;
            Engine::GetInstance().ui->fase2 = false;
            Engine::GetInstance().ui->fase3 = true;
        }

        transformStep = 0;
        transformTimer = 0.0f;
        transformationStarted = false;
        Hiteado = false;
        zooming = false;
        break;
    }
}

void Devil::ResizeCollisionForPhase3() {
    if (pbody) {
        // Store current position
        b2Vec2 currentPos = pbody->body->GetTransform().p;

        // Delete old physics body
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);

        // Create new rectangular collision body for phase 3
        int newWidth = texW + 140;   // Adjust width as needed
        int newHeight = texH + 350;  // Adjust height as needed

        pbody = Engine::GetInstance().physics.get()->CreateRectangle(
            METERS_TO_PIXELS(currentPos.x) ,
            METERS_TO_PIXELS(currentPos.y) - 90,
            newWidth,
            newHeight,
            bodyType::DYNAMIC
        );

        pbody->ctype = ColliderType::DEVIL;
        pbody->body->SetGravityScale(1.0f);
        pbody->listener = this;
        pbody->body->SetFixedRotation(true); // Prevent rotation for rectangular body
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
            METERS_TO_PIXELS(currentPos.y) - 90,
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
        punchX += isLookingLeft ? -20 : 20;

        punchAttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            punchX, punchY,
            50, 30,
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

    // Sprite offset adjustments for different phases and animations
    if (currentAnimation == &transform || currentPhase >= 2) {
        offsetY = -137;
        if (isLookingLeft) offsetX = -130;
        if (!isLookingLeft) offsetX = -100;
    }

    if (currentPhase == 3) {
        if (isLookingLeft) offsetX = -150;
    }
    if (currentAnimation == &idle2) {
        offsetY = -137;
    }

    if (currentAnimation == &salto || currentAnimation == &land) {
        offsetY = -425;
    }
    if (currentAnimation == &idle3 ) {
        offsetX = -130;
        offsetY = -240;
    }
    if (currentAnimation == &transform2 || currentAnimation == &attackH || currentAnimation == &attackV || currentAnimation == &death) {
        offsetY = -240;
    }
    if (currentAnimation == &attackH || currentAnimation == &attackV) {
        if (isLookingLeft)offsetX = -320;
        offsetY = -240;
        if (!isLookingLeft)offsetX = -110;
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
        LOG("Golpeado");
        LOG("Hiteado %s", Hiteado ? "true" : "false");
        if (!Hiteado && !isTransforming) {
            Hiteado = true; // Set hit flag immediately

            // Handle hits based on current phase
            if (currentPhase == 1) {
                LOG("Devil hit! Lives remaining: %d, Current Phase: %d", live1, currentPhase);
                live1--;
                if (live1 <= 0) {
                    isTransforming = true;

                    // Cancel any active attacks
                    if (isAttacking) {
                        isAttacking = false;
                        if (punchAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                            punchAttackArea = nullptr;
                        }
                        punch.Reset();
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
                    Hiteado = false;
                }
            }
            else if (currentPhase == 2) {
                live2--;
                if (live2 <= 0) {
                    // Establecer la transformaci�n como pendiente ANTES de crear el salto
                    pendingTransformation = true;

                    // Cancel any active attacks
                    if (isAttacking) {
                        isAttacking = false;
                        if (punchAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                            punchAttackArea = nullptr;
                        }
                    }

                    // Cancel tail attack if active
                    if (isTailAttacking) {
                        isTailAttacking = false;
                        if (tailAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                            tailAttackArea = nullptr;
                        }
                        colatazo.Reset();
                    }

					// Jump to x=1200 for transformation
                    if (!jumpAttackActive) {
                        CreateJumpAttack();
                    }

                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }
                else {
                    Hiteado = false;
                }
            }
            else if (currentPhase == 3) {
                live3--;
                if (live3 <= 0) {
                    isDying = true;
                    currentAnimation = &death;
                    currentAnimation->Reset();

                    // Cancel any active attacks
                    if (isAttacking) {
                        isAttacking = false;
                        if (punchAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                            punchAttackArea = nullptr;
                        }
                    }

                    // Cancel tail attack if active
                    if (isTailAttacking) {
                        isTailAttacking = false;
                        if (tailAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                            tailAttackArea = nullptr;
                        }
                        colatazo.Reset();
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

                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }
                else {
                    Hiteado = false;
                }
            }
        }
        break;

    case ColliderType::PLAYER_WHIP_ATTACK:
        if (!Hiteado && !isTransforming) {
            Hiteado = true; // Set hit flag immediately

            // Handle hits based on current phase
            if (currentPhase == 1) {
                live1 = live1 - 2;
                if (live1 <= 0) {
                    isTransforming = true;

                    // Cancel any active attacks
                    if (isAttacking) {
                        isAttacking = false;
                        if (punchAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                            punchAttackArea = nullptr;
                        }
                        punch.Reset();
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
                    Hiteado = false;
                }
            }
            else if (currentPhase == 2) {
                live2 = live2 - 2;
                if (live2 <= 0) {
                    pendingTransformation = true;

                    // Cancel any active attacks
                    if (isAttacking) {
                        isAttacking = false;
                        if (punchAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                            punchAttackArea = nullptr;
                        }
                    }

                    // Cancel tail attack if active
                    if (isTailAttacking) {
                        isTailAttacking = false;
                        if (tailAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                            tailAttackArea = nullptr;
                        }
                        colatazo.Reset();
                    }

                    if (!jumpAttackActive) {
                        CreateJumpAttack();
                    }

                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }
                else {
                    Hiteado = false;
                }
            }
            else if (currentPhase == 3) {
                live3 = live3 - 2;
                if (live3 <= 0) {
                    isDying = true;
                    currentAnimation = &death;
                    currentAnimation->Reset();

                    // Cancel any active attacks
                    if (isAttacking) {
                        isAttacking = false;
                        if (punchAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                            punchAttackArea = nullptr;
                        }
                    }

                    // Cancel tail attack if active
                    if (isTailAttacking) {
                        isTailAttacking = false;
                        if (tailAttackArea) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                            tailAttackArea = nullptr;
                        }
                        colatazo.Reset();
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

                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                }
                else {
                    Hiteado = false;
                }
            }
        }
        break;
        case ColliderType::PROJECTILE:
            if (!Hiteado && !isTransforming) {
                Hiteado = true; // Set hit flag immediately

                // Handle hits based on current phase
                if (currentPhase == 1) {
                    live1--;
                    if (live1 <= 0) {
                        isTransforming = true;

                        // Cancel any active attacks
                        if (isAttacking) {
                            isAttacking = false;
                            if (punchAttackArea) {
                                Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                                punchAttackArea = nullptr;
                            }
                            punch.Reset();
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
                        Hiteado = false;
                    }
                }
                else if (currentPhase == 2) {
                    live2--;
                    if (live2 <= 0) {
                        pendingTransformation = true;

                        // Cancel any active attacks
                        if (isAttacking) {
                            isAttacking = false;
                            if (punchAttackArea) {
                                Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                                punchAttackArea = nullptr;
                            }
                        }

                        // Cancel tail attack if active
                        if (isTailAttacking) {
                            isTailAttacking = false;
                            if (tailAttackArea) {
                                Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                                tailAttackArea = nullptr;
                            }
                            colatazo.Reset();
                        }

                        if (!jumpAttackActive) {
                            CreateJumpAttack();
                        }

                        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                    }
                    else {
                        Hiteado = false;
                    }
                }
                else if (currentPhase == 3) {
                    live3--;
                    if (live3 <= 0) {
                        isDying = true;
                        currentAnimation = &death;
                        currentAnimation->Reset();
                       
                        // Cancel any active attacks
                        if (isAttacking) {
                            isAttacking = false;
                            if (punchAttackArea) {
                                Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
                                punchAttackArea = nullptr;
                            }
                        }

                        // Cancel tail attack if active
                        if (isTailAttacking) {
                            isTailAttacking = false;
                            if (tailAttackArea) {
                                Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
                                tailAttackArea = nullptr;
                            }
                            colatazo.Reset();
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

                        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
                    }
                    else {
                        Hiteado = false;
                    }
                }
            }
            break;
             case ColliderType::BOSSFLOOR:
                 if (pbody && pbody->body) {
                     pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(822), PIXEL_TO_METERS(1538)), pbody->body->GetAngle());
                     LOG("Boss teleported to position (822, 1538)");
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
    if (tailAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
        tailAttackArea = nullptr;
    }
    if (pathfinding) {
        delete pathfinding;
        pathfinding = nullptr;
    }
    CleanupSpears();
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
    live1 = 3;//Live for phase 1
    live2 = 6;//Live for phase 2
    live3 = 8;//live for phase 3
    currentPhase = 1;
    currentAnimation = &idle;
    isDying = false;
    isTransforming = false;
    jumpAttackActive = false;
    isJumping = false;
    isLanding = false;
    shadowVisible = false;

    // Reset all attack states
    isTailAttacking = false;
    isAttacking = false;
    canAttack = true;
    currentAttackCooldown = 0.0f;

    // Clean up all attack areas
    if (punchAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(punchAttackArea);
        punchAttackArea = nullptr;
    }
    if (jumpAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(jumpAttackArea);
        jumpAttackArea = nullptr;
    }
    if (tailAttackArea) {
        Engine::GetInstance().physics.get()->DeletePhysBody(tailAttackArea);
        tailAttackArea = nullptr;
    }
}