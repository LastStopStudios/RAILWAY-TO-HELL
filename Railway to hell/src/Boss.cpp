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
    currentAnimation = &idle;

    //initialize enemy parameters 
    moveSpeed = 30.0f; 
    patrolSpeed = 30.0f;
    savedPosX = 0.0f;

    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

    pbody->listener = this;
    //Assign collider type
    pbody->ctype = ColliderType::BOSS;

    pbody->body->SetFixedRotation(false);
    pbody->body->SetGravityScale(1.0f);

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(2.0f);

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    a = 0;
	kill = 1;

    return true;
}

bool Boss::Update(float dt)
{

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
    if (isDying) {
        currentAnimation->Update();

        // If death animation finished, start the timer
        if (currentAnimation->HasFinished()) {
            deathTimer += dt;
            if (deathTimer >= deathDelay) {
                // Mark for destruction in the next frame
                pendingToDelete = true;
            }
        }

        // Draw the death animation
        SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY() - 32 , &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);

        // When dying, don't process any other logic
        return true;
    }


    enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    if (Engine::GetInstance().entityManager->dialogo == false) {

        b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

        float dx = playerTilePos.getX() - enemyTilePos.getX();
        float dy = playerTilePos.getY() - enemyTilePos.getY();
        // Calculate the distance between the enemy and the player only on the X axis
        float distanceToPlayer = abs(dx);

        // Limit movement towards the player only if within X tiles
        float patrolDistance = 7.0f; // Set the maximum distance for the enemy to chase the player 

        if (!canAttack) { // update the attack cooldown
            currentAttackCooldown -= dt;
            if (currentAttackCooldown <= 0) {
                canAttack = true;
                currentAttackCooldown = 0.0f;
            }
        }

        if ((isLookingLeft && dx <= -attackDistance)) {
            pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
        }

        isLookingLeft = dx < 0; // Set the direction of the enemy



        if (distanceToPlayer <= attackDistance && canAttack && !isAttacking) { // start attacking
            isAttacking = true;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation = &attack;
            currentAnimation->Reset();

            if (area == nullptr) { // create the attack area
                area = Engine::GetInstance().physics.get()->CreateRectangleSensor(
                    (int)position.getX() + (isLookingLeft ? -30 : 160),
                    (int)position.getY() + 50,
                    160, //modify according to the length of the whip
                    40,
                    bodyType::KINEMATIC
                );
                area->ctype = ColliderType::BOSS_ATTACK;
            }
        }

        if (isAttacking) {
            pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));

            if (attack.HasFinished()) { // stop attacking
                isAttacking = false;
                currentAnimation = &running;
                attack.Reset();


                if (area != nullptr) { // delete the attack area
                    Engine::GetInstance().physics.get()->DeletePhysBody(area);
                    area = nullptr;
                }
            }
        }

        if (!isAttacking) {
            if (distanceToPlayer <= patrolDistance) { // if player is within patrol distance
                if (!resting) { // start chasing player
                    resting = true;
                    currentAnimation = &running;
                    running.Reset();
                }

                if (currentAnimation == &running) { // pathfinding logic
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
                            float velocityX = isLookingLeft ? -moveSpeed : moveSpeed;
                            b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(velocityX), pbody->body->GetLinearVelocity().y);
                            pbody->body->SetLinearVelocity(velocity);
                        }
                    }
                }
            }
            else { // idle (not chasing player)
                if (resting) {
                    resting = false;
                    currentAnimation = &idle;
                    idle.Reset();
                }
                b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
                pbody->body->SetLinearVelocity(velocity);
            }
        }

        SDL_Rect frame = currentAnimation->GetCurrentFrame();
        offsetX = 0; // used to adjust the position of the sprite

        // change sprite direction
        if (isLookingLeft) {
            flip = SDL_FLIP_HORIZONTAL;
            offsetX = (frame.w - texW); //96-48 
        }
        else {
            flip = SDL_FLIP_NONE;
			
        }

        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - 32);

    }
    SDL_Rect frame = currentAnimation->GetCurrentFrame();

    int renderX = (int)position.getX() - offsetX;
    int renderY = (int)position.getY() + texH - 32 - frame.h; // Centrado vertical

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
    if (area != nullptr) {
        Engine::GetInstance().physics.get()->DeletePhysBody(area);
        area = nullptr;
    }

    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void Boss::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Boss::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Boss::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collided with player - DESTROY");
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;

    case ColliderType::PLAYER_ATTACK:
        if (!isDying) { // Prevent multiple death animations
            isDying = true;
            currentAnimation = &die;
            currentAnimation->Reset();

            pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors
            // Engine::GetInstance().audio.get()->PlayFx(deathFx);
        }
    case ColliderType::PLAYER_WHIP_ATTACK:
        if (!isDead) {
            isDead = true;
            currentAnimation = &die;
            a = 1;
            Engine::GetInstance().scene->DesbloquearSensor();//Unblock scene change sensors 
            // Detener movimiento del cuerpo físico
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            pbody->body->SetGravityScale(0.0f); // Por si está cayendo
        }
        break;

    }
}

void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        LOG("Collision player");
        break;
    }
}