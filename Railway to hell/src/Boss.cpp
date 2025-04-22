#include "Boss.h"
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
#include "DialogoM.h"


Boss::Boss() : Entity(EntityType::BOSS)
{

}

Boss::~Boss() {
    delete pathfinding;
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
    }
}
bool Boss::Awake() {
    return true;
}

bool Boss::Start() {

    //initilize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    attack.LoadAnimations(parameters.child("animations").child("whipAttack"));
    running.LoadAnimations(parameters.child("animations").child("walking"));
    die.LoadAnimations(parameters.child("animations").child("die"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    currentAnimation = &idle;

    dialogTriggered = false;
    battleStarted = false;
    isLookingLeft = true;

    // Start with idle animation until dialog is shown
    currentAnimation = &idle;

    //initialize enemy parameters 
    moveSpeed = 30.0f;
    patrolSpeed = 30.0f;
    savedPosX = 0.0f;
    // Physics body initialization - now with two circular collisions
    int radius = texW / 3.1; // Smaller radius than before
    int centerX = (int)position.getX() + texW / 2;
    int centerY = (int)position.getY() + texH / 2;
    // Place upper body higher up
    pbodyUpper = Engine::GetInstance().physics.get()->CreateCircle(
        centerX,
        centerY - radius,  // upper part
        radius,
        bodyType::DYNAMIC);
    pbodyUpper->listener = this;
    pbodyUpper->ctype = ColliderType::BOSS;
    pbodyUpper->body->SetFixedRotation(true);
    // Lower body (legs)
    pbodyLower = Engine::GetInstance().physics.get()->CreateCircle(
        centerX,
        centerY + radius,  // lower part
        radius,
        bodyType::DYNAMIC);
    pbodyLower->listener = this;
    pbodyLower->ctype = ColliderType::BOSS;
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

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    a = 0;
    kill = 1;

    return true;
}

void Boss::TriggerBossDialog() {
    dialogTriggered = true;
}

bool Boss::Update(float dt)
{
    if (!battleStarted) {
        // Check if boss dialog has been shown and closed
        if (Engine::GetInstance().dialogoM->bossFightReady) {
            battleStarted = true;
            // Immediately set looking left and running animation for initial walk
            isLookingLeft = true;
            currentAnimation = &running;
            running.Reset();
        }
        else {
            // Keep boss in idle until battle begins
            currentAnimation = &idle;

            // Make sure boss doesn't move while waiting
            if (pbodyUpper != nullptr && pbodyUpper->body != nullptr) {
                pbodyUpper->body->SetLinearVelocity(b2Vec2(0, 0));
            }
            if (pbodyLower != nullptr && pbodyLower->body != nullptr) {
                pbodyLower->body->SetLinearVelocity(b2Vec2(0, 0));
            }

            // Calculate the frame and offset the same way as in the main animation code
            SDL_Rect frame = currentAnimation->GetCurrentFrame();

            // Initialize offsetX
            offsetX = 0;

            // Set flip direction consistently even during idle state
            if (isLookingLeft) {
                flip = SDL_FLIP_HORIZONTAL;
                offsetX = (frame.w - texW);
            }
            else {
                flip = SDL_FLIP_NONE;
            }

            int renderX = (int)position.getX() - offsetX;
            int renderY = (int)position.getY() + texH - 25 - frame.h; // Vertical centering

            Engine::GetInstance().render.get()->DrawTexture(
                texture,
                renderX,
                renderY,
                &frame,
                1.0f,
                0.0,
                INT_MAX,
                INT_MAX,
                flip
            );

            currentAnimation->Update();
            return true; // Skip the rest of the logic until battle starts
        }
    }

    enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    // FIX: Check if hurt animation is complete and reset to appropriate animation
    if (ishurt) {
        if (hurt.HasFinished()) {
            ishurt = false;
            // Return to the appropriate animation state
            if (isAttacking) {
                currentAnimation = &attack;
            }
            else {
                currentAnimation = &running; // Always go to running when not attacking
            }
        }
    }

    if (!ishurt && !isDying && !isDead) {
        if (Engine::GetInstance().entityManager->dialogo == false) {
            // Get current velocity of upper body
            b2Vec2 velocityUpper = pbodyUpper->body->GetLinearVelocity();
            b2Vec2 velocityLower = pbodyLower->body->GetLinearVelocity();

            // Verify if we should do the initial walk
            if (!initialWalkComplete) {
                initialWalkTimer += dt;

                // Start walking animation
                if (currentAnimation != &running) {
                    currentAnimation = &running;
                    running.Reset();
                }

                // Force looking left for initial walk
                isLookingLeft = true;

                // Always move left for initial walk
                float velocityX = -moveSpeed; // Move left
                pbodyUpper->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(velocityX), velocityUpper.y));
                pbodyLower->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(velocityX), velocityLower.y));

                // Check if walk time is finished
                if (initialWalkTimer >= initialWalkDuration) {
                    initialWalkComplete = true;
                    currentAnimation = &running; // Changed from idle to running to be consistent
                }
            }
            else {
                // Normal boss logic after initial walk
                float dx = playerTilePos.getX() - enemyTilePos.getX();
                float dy = playerTilePos.getY() - enemyTilePos.getY();

                // Set looking direction based on player position
                isLookingLeft = dx < 0;

                // Attack cooldown management
                if (!canAttack) {
                    currentAttackCooldown -= dt;
                    if (currentAttackCooldown <= 0) {
                        canAttack = true;
                        currentAttackCooldown = 0.0f;
                    }
                }

                // Check for attack condition - using absolute distance for flexibility
                float distanceToPlayer = abs(dx);

                // If in attack range and can attack, start attacking
                if (distanceToPlayer <= attackDistance && canAttack && !isAttacking) {
                    isAttacking = true;
                    canAttack = false;
                    currentAttackCooldown = attackCooldown;
                    currentAnimation = &attack;
                    currentAnimation->Reset();

                    // Create attack area if it doesn't exist
                    if (area == nullptr) {
                        area = Engine::GetInstance().physics.get()->CreateRectangleSensor(
                            (int)position.getX() + (isLookingLeft ? -30 : 160),
                            (int)position.getY() + 50,
                            160, // adjust according to whip length
                            40,
                            bodyType::KINEMATIC
                        );
                        area->ctype = ColliderType::BOSS_ATTACK;
                    }
                }

                // Handle attack state
                if (isAttacking) {
                    // Stay still while attacking
                    pbodyUpper->body->SetLinearVelocity(b2Vec2(0, velocityUpper.y));
                    pbodyLower->body->SetLinearVelocity(b2Vec2(0, velocityLower.y));

                    // Check if attack animation is finished
                    if (attack.HasFinished()) {
                        isAttacking = false;
                        currentAnimation = &running;
                        attack.Reset();

                        // Delete attack area after attack
                        if (area != nullptr) {
                            Engine::GetInstance().physics.get()->DeletePhysBody(area);
                            area = nullptr;
                        }
                    }
                }
                else {
                    // Always chase player with pathfinding regardless of distance
                    if (currentAnimation != &running) {
                        currentAnimation = &running;
                        running.Reset();
                    }

                    // Pathfinding logic - always active now
                    int maxIterations = 100;
                    int iterations = 0;

                    while (pathfinding->pathTiles.empty() && iterations < maxIterations) {
                        pathfinding->PropagateAStar(SQUARED);
                        iterations++;
                    }

                    if (pathfinding->pathTiles.size() >= 2) {
                        auto it = pathfinding->pathTiles.end();
                        --it;
                        --it;
                        Vector2D nextTile = *it;
                        Vector2D nextPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

                        float x2 = nextPos.getX();
                        float x1 = enemyPos.getX();
                        float dx = x2 - x1;
                        float distance = abs(dx);

                        if (distance < 5.0f) {
                            pathfinding->pathTiles.pop_back();
                        }
                        else {
                            // Adjust movement direction based on path
                            bool shouldMoveLeft = x2 < x1;
                            isLookingLeft = shouldMoveLeft;

                            float velocityX = shouldMoveLeft ? -moveSpeed : moveSpeed;
                            // Apply velocity to both bodies to follow path
                            pbodyUpper->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(velocityX), velocityUpper.y));
                            pbodyLower->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(velocityX), velocityLower.y));
                        }
                    }
                    else {
                        // If no path found, keep running animation but stop moving
                        pbodyUpper->body->SetLinearVelocity(b2Vec2(0, velocityUpper.y));
                        pbodyLower->body->SetLinearVelocity(b2Vec2(0, velocityLower.y));
                    }
                }
            }

            SDL_Rect frame = currentAnimation->GetCurrentFrame();
            offsetX = 0; // used to adjust sprite position

            // change sprite direction
            if (isLookingLeft) {
                flip = SDL_FLIP_HORIZONTAL;
                offsetX = (frame.w - texW); //96-48 
            }
            else {
                flip = SDL_FLIP_NONE;
            }

            // Update position from the upper body's transform
            b2Transform pbodyUpperPos = pbodyUpper->body->GetTransform();
            position.setX(METERS_TO_PIXELS(pbodyUpperPos.p.x) - texH / 2);
            position.setY(METERS_TO_PIXELS(pbodyUpperPos.p.y) - 32);
        }
    }

    SDL_Rect frame = currentAnimation->GetCurrentFrame();

    int renderX = (int)position.getX() - offsetX + 16;
    int renderY = (int)position.getY() + texH - 5 - frame.h; // Vertical centering

    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        renderX,
        renderY,
        &frame,
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        flip
    );

    currentAnimation->Update();

    // pathfinding drawing
    pathfinding->DrawPath();
    pathfinding->ResetPath(enemyTilePos);

    return true;
}
void Boss::Matar() {//eliminating the enemy once dead 
    if (kill == 1) {
        kill = 2;
        Disable();//when it has to be activated use  Enable();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
}

bool Boss::CleanUp()
{
    // First delete all physics bodies
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
        area = nullptr;
    }

    if (pbodyUpper != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyUpper);
        pbodyUpper = nullptr;
    }

    if (pbodyLower != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(pbodyLower);
        pbodyLower = nullptr;
    }

    // Set joint to null after deleting bodies
    bodyJoint = nullptr;

    // Make sure textures are properly released
    // We don't need to destroy the texture as it's managed elsewhere
    // Just set our pointer to null to avoid double deletion issues
    texture = nullptr;

    return true;
}

void Boss::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));

    // Calculate the current offset between upper and lower bodies
    b2Vec2 upperPos = pbodyUpper->body->GetPosition();
    b2Vec2 lowerPos = pbodyLower->body->GetPosition();
    b2Vec2 offset = lowerPos - upperPos;

    // Set position for upper body
    pbodyUpper->body->SetTransform(bodyPos, 0);

    // Set position for lower body, maintaining the same offset
    pbodyLower->body->SetTransform(bodyPos + offset, 0);
}

Vector2D Boss::GetPosition() {
    // We use the upper body for position tracking
    b2Vec2 bodyPos = pbodyUpper->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Boss::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

// Fix for hurt animation in OnCollision
void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
    // First check if the collision involves one of our bodies
    if (physA != pbodyUpper && physA != pbodyLower) {
        return;
    }

    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        // Player collision handling
        break;

    case ColliderType::PLAYER_ATTACK: {
        if (lives > 0) {
            lives--;
            if (lives > 0) {
                ishurt = true;
                currentAnimation = &hurt;
                hurt.Reset();
            }
            else if (lives <= 0 && !isDying) { // Prevent multiple death animations
                isDying = true;
                currentAnimation = &die;
                currentAnimation->Reset();

                // Stop both bodies
                pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors
                Engine::GetInstance().dialogoM->Texto("2");//text after boss death
                // Engine::GetInstance().audio.get()->PlayFx(deathFx);
            }
        }
    }
                                    break;

    case ColliderType::PLAYER_WHIP_ATTACK: {
        if (lives > 0) {
            lives -= 1;  //It should be - 2, 6 - 2 = 4 so the boss should die on the third hit but for some reason if i do it like that he dies in two hits
            if (lives > 0) {
                ishurt = true;
                currentAnimation = &hurt;
                hurt.Reset();
            }
            else if (lives <= 0 && !isDead) {
                isDead = true;
                currentAnimation = &die;
                a = 1;
                Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors
                Engine::GetInstance().dialogoM->Texto("2");//text after boss death 

                // Stop both bodies
                pbodyUpper->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                pbodyLower->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

                // Disable gravity on both bodies
                pbodyUpper->body->SetGravityScale(0.0f);
                pbodyLower->body->SetGravityScale(0.0f);
            }
        }
    }
                                         break;
    }
}

void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    // Check if the collision involves one of our bodies
    if (physA != pbodyUpper && physA != pbodyLower) {
        return;
    }

    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        break;
    }
}