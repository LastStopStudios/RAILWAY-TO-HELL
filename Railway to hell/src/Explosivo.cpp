#include "Explosivo.h"
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

Explosivo::Explosivo() : Entity(EntityType::EXPLOSIVO)
{

}

Explosivo::~Explosivo() {
    delete pathfinding;
}

bool Explosivo::Awake() {
    return true;
}

bool Explosivo::Start() {

    // Initialize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    die.LoadAnimations(parameters.child("animations").child("die"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    explode.LoadAnimations(parameters.child("animations").child("explode"));
    spot.LoadAnimations(parameters.child("animations").child("spot"));
    run.LoadAnimations(parameters.child("animations").child("run"));

    currentAnimation = &idle;

    // Add physics to entity - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2.5, bodyType::DYNAMIC);

    // Assign collider type
    pbody->ctype = ColliderType::AMEGO;

    pbody->listener = this;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    vez = 1;
    ResetPath();
    a = 0;
    kill = 1;
    isSpotting = false;  // Variable to control spot animation
    hasSpotted = false;  // Variable to track if spotting animation has been done
    return true;
}

bool Explosivo::Update(float dt)
{
    // If exploding, apply offset immediately
    if (isExploding) {
        // Unified death/explosion state handling
        // Ensure that there is no movement during death/explosion
        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }

        // Update the current animation
        currentAnimation->Update();

        // If animation finished, start the timer for deletion
        if (currentAnimation->HasFinished()) {
            deathTimer += dt;
            LOG("Death Timer: %f of %f", deathTimer, deathDelay);
            if (deathTimer >= deathDelay) {
                LOG("Setting pendingToDelete = true");
                pendingToDelete = true;
            }
        }

        // Draw the death/explosion animation with offset
        SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

        // Always apply offset for explosion
        float yOffset = -explodeOffsetY; // Negative because we want to draw it higher

        Engine::GetInstance().render.get()->DrawTexture(
            texture,
            (int)position.getX() - 32,
            (int)position.getY() - 32 + yOffset,  // Apply offset here
            &currentAnimation->GetCurrentFrame(),
            1.0f, 0.0, INT_MAX, INT_MAX,
            flip
        );
        // Handle damage to player if explosion and collision happened
        if (exploto && toco) {
            Engine::GetInstance().scene->hitearPlayer();
            exploto = false;
        }

        // When dying/exploding, don't process any other logic
        return true;
    }

    // Don't process logic if we're not in GAMEPLAY mode
    bool isGameplay = Engine::GetInstance().scene->GetCurrentState() == SceneState::GAMEPLAY;

    if (!isGameplay) {
        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }
        return true;
    }
    else {
        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetGravityScale(1.0f);
        }
    }

    if (ishurt) {
        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }
        if (hurt.HasFinished()) {
            ishurt = false;
            currentAnimation = &idle;
        }
    }

    // Unified death/explosion state handling for dying state
    if (isDying) {
        // Ensure that there is no movement during death/explosion
        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }

        // Update the current animation
        currentAnimation->Update();

        // If animation finished, start the timer for deletion
        if (currentAnimation->HasFinished()) {
            deathTimer += dt;
            LOG("Death Timer: %f of %f", deathTimer, deathDelay);
            if (deathTimer >= deathDelay) {
                LOG("Setting pendingToDelete = true");
                pendingToDelete = true;
            }
        }

        // Draw the death animation (without offset)
        SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

        Engine::GetInstance().render.get()->DrawTexture(
            texture,
            (int)position.getX() - 32,
            (int)position.getY() - 32,
            &currentAnimation->GetCurrentFrame(),
            1.0f, 0.0, INT_MAX, INT_MAX,
            flip
        );

        // When dying/exploding, don't process any other logic
        return true;
    }

    // Self-explosion timer logic
    if (cigarro) {
        explosiveTimer += dt;
        LOG("Timer: %f", explosiveTimer);

        if (explosiveTimer >= Ivolo && !isExploding) {
            // Start explosion sequence
            isExploding = true;
            currentAnimation = &explode;
            currentAnimation->Reset();
            deathTimer = 0.0f;

            // Stop movement
            if (pbody != nullptr && pbody->body != nullptr) {
                pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                pbody->body->SetAwake(false);
                pbody->body->SetGravityScale(0.0f);
            }

            // Set explosion state
            exploto = true;

            // Update position once to apply offset immediately
            // This prevents the first frame from showing without offset
            position.setY(position.getY());
            return Update(dt); // Re-run Update to apply offset immediately
        }
    }

   

    // Handle skipping first input if necessary
    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();
        return true;
    }

    if (Engine::GetInstance().entityManager->dialogo == false) {

        // Constants to adjust enemy behavior
        const float DETECTION_DISTANCE = 250.0f;
        const float CHASE_SPEED = 140.0f;
        const float PATROL_SPEED = 50.0f;
        const int MAX_PATHFINDING_ITERATIONS = 50;
        const int TikingDistance = 80;

        // Get current positions
        enemyPos = GetPosition();
        Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();

        // Convert to tile coordinates
        Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
        Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

        // Calculate distance to player (Euclidean distance)
        float dx = playerPos.getX() - enemyPos.getX();
        float dy = playerPos.getY() - enemyPos.getY();
        float distanceToPlayer = sqrt(dx * dx + dy * dy);

        // Variable to control speed based on mode
        float velocityX = 0.0f;
        bool wasChasing = isChasing; // Store previous state
        isChasing = false;

        // Spot animation handling (new logic)
        if (!ishurt && !isSpotting) {
            // Player detected but not yet spotted (first detection)
            if (distanceToPlayer <= DETECTION_DISTANCE && !hasSpotted) {
                isSpotting = true;
                hasSpotted = true;
                currentAnimation = &spot;
                currentAnimation->Reset();
                LOG("FIRST SPOT");
                // Stop movement during spotting animation
                if (pbody != nullptr && pbody->body != nullptr) {
                    pbody->body->SetLinearVelocity(b2Vec2(0, 0));
                }

                // Set direction to face player
                isLookingLeft = (dx < 0);
            }
        }

        // Check if spot animation has finished
        if (isSpotting) {
            if (currentAnimation->HasFinished()) {
                isSpotting = false;
                currentAnimation = &run;  // Switch to run animation for chasing
                currentAnimation->Reset();
                LOG("CURRENT ANIM FINISH SPOTING FALSE"); 
            }
            else {
                // Keep updating spot animation but don't move
                currentAnimation->Update();
                LOG("CURRENT ANIM NOT FINISH SPOTING FALSE");
                // Configure sprite flip based on direction
                SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

                // Draw enemy texture and animation
                Engine::GetInstance().render.get()->DrawTexture(
                    texture,
                    (int)position.getX(),
                    (int)position.getY(),
                    &currentAnimation->GetCurrentFrame(),
                    1.0f, 0.0, INT_MAX, INT_MAX,
                    flip
                );

                return true;  // Skip the rest of the update during spot animation
            }
        }

        // Chase mode (high priority)
        if (!ishurt && !isSpotting) {
            if (distanceToPlayer <= DETECTION_DISTANCE)
            {
                isChasing = true;
                LOG("Distance: %f", distanceToPlayer);

                // Use run animation during chase
                if (currentAnimation != &run && !cigarro) {
                    currentAnimation = &run;
                    currentAnimation->Reset(); // Ensure animation resets
                }

                if (distanceToPlayer < TikingDistance) {
                    LOG("Tiking: %s", cigarro ? "true" : "false");
                    cigarro = true;
                    // Switch to explode animation will happen when timer expires
                }

                if (distanceToPlayer >= 240) {
                    cigarro = false;
                    exploto = false;
                    explosiveTimer = 0.0f;
                    // Reset to idle if not chasing
                    if (currentAnimation != &idle) {
                        currentAnimation = &idle;
                        currentAnimation->Reset(); // Ensure animation resets
                    }
                }//Reset timer since player is far away

                // Reset and calculate path to player
                pathfinding->ResetPath(enemyTilePos);

                // Run A* algorithm with iteration limit
                for (int i = 0; i < MAX_PATHFINDING_ITERATIONS; i++) {
                    pathfinding->PropagateAStar(SQUARED);
                    if (pathfinding->ReachedPlayer(playerTilePos)) {
                        break;
                    }
                }

                // Compute path from origin to destination
                pathfinding->ComputePath(playerTilePos.getX(), playerTilePos.getY());

                // Check if a valid path was found
                if (!pathfinding->pathTiles.empty() && pathfinding->pathTiles.size() > 1)
                {
                    // Get the next point in the path
                    // To ensure consistency, always use the second point in the path
                    Vector2D nextTile = *(std::next(pathfinding->pathTiles.begin(), 1));

                    // Convert tile position to world coordinates
                    Vector2D nextPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

                    // Determine movement direction with greater precision
                    float moveX = nextPos.getX() - enemyPos.getX();

                    // More precise threshold to determine direction
                    if (moveX < -1.0f) {
                        isLookingLeft = true;
                    }
                    else if (moveX > 1.0f) {
                        isLookingLeft = false;
                    }

                    // Adjust speed according to direction
                    velocityX = isLookingLeft ? -CHASE_SPEED : CHASE_SPEED;
                }
            }
            else {
                // Smooth transition from chase to patrol
                if (wasChasing && !cigarro) {
                    // Just stopped chasing
                    currentAnimation = &idle;
                    currentAnimation->Reset();
                }
                // Return to patrolling behavior
                if (currentAnimation != &idle) {
                    currentAnimation = &idle;
                    currentAnimation->Reset(); // Ensure animation resets correctly
                }

                hasSpotted = false;  // Reset spot status when player is out of range

                if (vez < 3) {
                    velocityX = giro ? PATROL_SPEED : -PATROL_SPEED;
                    isLookingLeft = !giro;
                }
                else {
                    bool isOutsidePatrolBounds = (enemyPos.getX() < std::min(patrol1, patrol2) || enemyPos.getX() > std::max(patrol1, patrol2));
                    if (isOutsidePatrolBounds) {
                        float patrolCenter = (patrol1 + patrol2) / 2.0f;
                        float direction = (patrolCenter > enemyPos.getX()) ? 1.0f : -1.0f;
                        velocityX = direction * PATROL_SPEED;
                        isLookingLeft = (direction < 0);

                        if (!isOutsidePatrolBounds) {
                            giro = (enemyPos.getX() < patrolCenter);
                        }
                    }
                    else {
                        // Patrol mode (low priority) - only if not chasing
                        velocityX = giro ? PATROL_SPEED : -PATROL_SPEED;
                        isLookingLeft = !giro;
                    }
                }
            }
        }

        // Apply velocity to physical body
        b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(velocityX), pbody->body->GetLinearVelocity().y);
        pbody->body->SetLinearVelocity(velocity);

        // Update position for rendering
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
    }
    else {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f)); // Stop the body
        currentAnimation = &idle;
    }

    // Configure sprite flip based on direction
    SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Draw enemy texture and animation
    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX(),
        (int)position.getY(),
        &currentAnimation->GetCurrentFrame(),
        1.0f, 0.0, INT_MAX, INT_MAX,
        flip
    );

    // Update animation - update only once
    if (!isExploding && !isDying) {
        currentAnimation->Update();
    }

    // Draw path for debugging
    // Ensure it's drawn in chase mode regardless of direction
    if (isChasing && !pathfinding->pathTiles.empty()) {
        pathfinding->DrawPath();
    }

    return true;
}

void Explosivo::Matar() { // Eliminate the enemy once dead 
    if (kill == 1) {
        kill = 2;
        Disable(); // When it has to be activated use Enable()
    }
}

bool Explosivo::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    vez = 1;
    return true;
}

void Explosivo::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Explosivo::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Explosivo::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Explosivo::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collided with player - DESTROY");
        toco = true;
        break;
    case ColliderType::PLAYER_ATTACK: {
        if (lives > 0) {
            lives--;
            if (lives > 0) {
                currentAnimation = &hurt;
                hurt.Reset();
                ishurt = true;
            }
            else if (lives <= 0 && !isDying && !isExploding) {
                // Start die animation when killed by player attack
                LOG("Starting die animation");
                isDying = true;
                currentAnimation = &die;
                currentAnimation->Reset();
                deathTimer = 0.0f;

                // Stop physical body
                if (pbody != nullptr && pbody->body != nullptr) {
                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    pbody->body->SetAwake(false);
                    pbody->body->SetGravityScale(0.0f);
                }

                // Disable cigar timer if it was active
                cigarro = false;
                explosiveTimer = 0.0f;
            }
        }
    }
                                    break;
    case ColliderType::PLAYER_WHIP_ATTACK: {
        if (lives > 0) {
            lives = lives - 2;
            if (lives > 0) {
                currentAnimation = &hurt;
                hurt.Reset();
                ishurt = true;
            }
            else if (lives <= 0 && !isDying && !isExploding) {
                // Start die animation when killed by whip attack
                LOG("Starting die animation from whip");
                isDying = true;
                currentAnimation = &die;
                currentAnimation->Reset();
                deathTimer = 0.0f;
                a = 1;

                // Stop physical body
                if (pbody != nullptr && pbody->body != nullptr) {
                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    pbody->body->SetAwake(false);
                    pbody->body->SetGravityScale(0.0f);
                }

                // Disable cigar timer if it was active
                cigarro = false;
                explosiveTimer = 0.0f;
            }
        }
        break;
    }
    case ColliderType::PROJECTILE: {
        if (lives > 0) {
            lives--;
            if (lives > 0) {
                currentAnimation = &hurt;
                hurt.Reset();
                ishurt = true;
            }
            else if (lives <= 0 && !isDying && !isExploding) {
                // Start die animation when killed by projectile
                LOG("Starting die animation from projectile");
                isDying = true;
                currentAnimation = &die;
                currentAnimation->Reset();
                deathTimer = 0.0f;

                // Stop physical body
                if (pbody != nullptr && pbody->body != nullptr) {
                    pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    pbody->body->SetAwake(false);
                    pbody->body->SetGravityScale(0.0f);
                }

                // Disable cigar timer if it was active
                cigarro = false;
                explosiveTimer = 0.0f;
            }
        }
        break;
    }
    case ColliderType::GIRO:
        giro = !giro;
        if (vez == 1) {
            Vector2D pos = GetPosition();
            patrol1 = pos.getX();
            vez++;
        }
        else if (vez == 2) {
            Vector2D pos = GetPosition();
            patrol2 = pos.getX();
            vez++;
        }
        break;
    }
}

void Explosivo::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        toco = false;
        break;
    }
}