#include "Volador.h"
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
Volador::Volador() : Entity(EntityType::VOLADOR)
{

}

Volador::~Volador() {
    delete pathfinding;
}

bool Volador::Awake() {
    return true;
}

bool Volador::Start() {

    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());

    // Obtain real dimensions of the texture
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

    // Use the texture dimensions or the original values as a backing
    texW = textureWidth > 0 ? textureWidth : parameters.attribute("w").as_int();
    texH = textureHeight > 0 ? textureHeight : parameters.attribute("h").as_int();

    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texIsDeath = parameters.attribute("isDeath").as_int();

    // Calculate collision radius based on texture size
    int collisionRadius = 32;
    texRadius = collisionRadius;

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
	die.LoadAnimations(parameters.child("animations").child("die"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    currentAnimation = &idle;

    moveSpeed = 30.0f; // Lateral movement speed
    minX = position.getX() - 130; // Left limit
    maxX = position.getX() + 50;  // Right limit
    movingRight = true; // Starts moving to the right
    chasingPlayer = false; // Initially not chasing the player

    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle(
        (int)position.getX() + texH / 2,
        (int)position.getY() + texH / 2,
        collisionRadius,
        bodyType::DYNAMIC
    );

    //Assign collider type
    pbody->ctype = ColliderType::VOLADOR;

    pbody->listener = this;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0.0f);

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();
    a = 0;
    kill = 1;
    Chasing = false;
    vez = 1;
    return true;
}

bool Volador::Update(float dt) {
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
            pbody->body->SetGravityScale(0.0f); 
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

    //The basic structure is that first the animation is performed and when it finishes, pendingToDelete is set to true, there is a function in entity.h that has this boolean variable
    // and there is also a function that returns the variable, in the entity manager in the update function logic has been added to check the variable and if it is set to true it destroys it there.
    // 
    // Handle death animation
    if (isDying || isDead) {
        // Ensure that there is no movement during death
        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f);
        }

        currentAnimation->Update();

        // If death animation finished, start the timer
        if (currentAnimation->HasFinished() && isDying || currentAnimation->HasFinished() && isDead) {
            deathTimer += dt;
            if (deathTimer >= deathDelay) {
                pendingToDelete = true;
            }
        }

        // Draw the death animation
        SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + 64, (int)position.getY() + 64, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);

        // When dying, don't process any other logic
        return true;
    }

    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();  //  We only do it once
        return true;
    }
    if (Engine::GetInstance().entityManager->dialogo == false)/*Check if the dialogue to stop the enemy has been activated.*/ {
    Vector2D camPos(Engine::GetInstance().render->camera.x, Engine::GetInstance().render->camera.y);
    Vector2D camSize(Engine::GetInstance().render->camera.w, Engine::GetInstance().render->camera.h);

    // Convert enemy position to camera coordinates
    Vector2D enemyScreenPos = enemyPos - camPos;

   
    Vector2D enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    float distanceToPlayerX = abs(playerTilePos.getX() - enemyTilePos.getX());
    const float patrolDistance = 7.0f;
    static bool movingRight = true;

    // Patrol and pathfinding logic
    if (!ishurt) {
        if (distanceToPlayerX <= patrolDistance) {
            // Pathfinding and moving towards the player
            Chasing = true;
            int maxIterations = 100;
            int iterations = 0;

            while (pathfinding->pathTiles.empty() && iterations < maxIterations) {
                pathfinding->PropagateAStar(SQUARED);
                iterations++;
            }

            if (!pathfinding->pathTiles.empty()) {
                auto it = pathfinding->pathTiles.end();
                --it; --it;
                Vector2D nextTile = *it;
                Vector2D nextPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());
                float dx = nextPos.getX() - enemyPos.getX();
                float dy = nextPos.getY() - enemyPos.getY();
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < 5.0f) {
                    pathfinding->pathTiles.pop_back();
                }
                else {
                    float stepX = (dx / distance) * moveSpeed;
                    float stepY = (dy / distance) * moveSpeed;
                    isLookingLeft = (dx < 0);
                    b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(stepX), PIXEL_TO_METERS(stepY));
                    pbody->body->SetLinearVelocity(velocity);
                }
            }
        }
        else {
            Chasing = false;
            const float patrolSpeed = 20.0f;

            // Only patrol if we already have both sensors
            if (vez >= 3) {
                Vector2D currentPos = GetPosition();

                // Calculate patrol centre and boundaries
                float minPatrolX = std::min(patrol1x, patrol2x);
                float maxPatrolX = std::max(patrol1x, patrol2x);
                float targetY = (patrol1y + patrol2y) / 2.0f;

                // Determine if outside the patrol area
                bool isOutsideX = (currentPos.getX() < minPatrolX || currentPos.getX() > maxPatrolX);
                bool isWrongHeight = fabs(currentPos.getY() - targetY) > 20.0f;

                // If outside the zone in X or Y
                if (isOutsideX || isWrongHeight) {
                    // adjust height if necessary
                    if (isWrongHeight) {
                        float dirY = (targetY > currentPos.getY()) ? 1.0f : -1.0f;
                        b2Vec2 velocity(0, PIXEL_TO_METERS(dirY * patrolSpeed * 0.7f));
                        pbody->body->SetLinearVelocity(velocity);
                    }
                    // adjust horizontal position
                    else if (isOutsideX) {
                        float targetX = (currentPos.getX() < minPatrolX) ? minPatrolX : maxPatrolX;
                        float dirX = (targetX > currentPos.getX()) ? 1.0f : -1.0f;
                        isLookingLeft = (dirX < 0);
                        b2Vec2 velocity(PIXEL_TO_METERS(dirX * patrolSpeed), 0);
                        pbody->body->SetLinearVelocity(velocity);
                    }
                }
                else {
                    // Normal patrol between the points
                    float targetX = giro ? patrol2x : patrol1x;
                    float dirX = (targetX > currentPos.getX()) ? 1.0f : -1.0f;
                    isLookingLeft = (dirX < 0);

                    b2Vec2 velocity(PIXEL_TO_METERS(dirX * patrolSpeed),0 );
                    pbody->body->SetLinearVelocity(velocity);

                    // Change direction if you reached the point
                    if (fabs(currentPos.getX() - targetX) < 15.0f) {
                        giro = !giro;
                    }
                }
            }
            else {
                // Initial behaviour until both sensors are detected
                const float initialPatrolSpeed = 20.0f;
                if (giro) {
                    b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(initialPatrolSpeed), 0.0f);
                    pbody->body->SetLinearVelocity(velocity);
                    isLookingLeft = false;
                }
                else {
                    b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(-initialPatrolSpeed), 0.0f);
                    pbody->body->SetLinearVelocity(velocity);
                    isLookingLeft = true;
                }
            }


        }
    }
        pathfinding->DrawPath();
        pathfinding->ResetPath(enemyTilePos);

    }else { pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));/*stop body*/ currentAnimation = &idle;}
    // Flip sprite configuration
    SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // Adjust position for rendering
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Draw texture and animation
    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + 64, (int)position.getY() + 64, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);

    currentAnimation->Update();

    if (isDead  && currentAnimation->HasFinished()) { Matar(); a = 2; }

    return true;
}

void Volador::Matar(){//eliminating the enemy once dead 
    if (kill == 1) {
        kill = 2;
        Disable();//when it has to be activated use  Enable();
       //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
}

bool Volador::CleanUp()
{

    if(texture){
        Engine::GetInstance().textures.get()->UnLoad(texture);
        texture = nullptr;
    }
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    vez = 1;
    Chasing = false;
    return true;
}

void Volador::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Volador::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Volador::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Volador::OnCollision(PhysBody* physA, PhysBody* physB) {

    Player* player = Engine::GetInstance().scene.get()->GetPlayer();

    switch (physB->ctype) {
    case ColliderType::PLAYER:
		
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::PLAYER_ATTACK: {
        if (lives > 0) {
            lives--;
            if (lives > 0) {
                currentAnimation = &hurt;
                hurt.Reset();
                ishurt = true;
            }
            else if (lives <= 0 && !isDying) { // Prevent multiple death animations
                isDying = true;
                currentAnimation = &die;
                currentAnimation->Reset();

                pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

                // Engine::GetInstance().audio.get()->PlayFx(deathFx);
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
            else if (lives <= 0 && !isDead) {
                isDead = true;
                currentAnimation = &die;
                a = 1;
                // Stop physical body movement
                pbody->body->SetLinearVelocity(b2Vec2(0, 0));
                pbody->body->SetGravityScale(0.0f); // In case it's falling
            }
        }

       
        
    }
		
		break;
    case ColliderType::GIRO:
        giro = !giro;
        if (vez == 1) {//First sensor
            Vector2D pos = GetPosition();//Right sensor position
            patrol1x = pos.getX();
            patrol1y = pos.getY();
            vez++;
        }
        else if (vez == 2) {//Second sensor
            Vector2D pos = GetPosition();//Left sensor position
            patrol2x = pos.getX();
            patrol2y = pos.getY();
            vez++;
        }
        break;

    default:
        break;
        }
}


void Volador::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype) {
    case ColliderType::PLATFORM:

        if (!isAttacking) {
            currentAnimation = &idle;
        }
        break;

    default:
        break;
    }
}

