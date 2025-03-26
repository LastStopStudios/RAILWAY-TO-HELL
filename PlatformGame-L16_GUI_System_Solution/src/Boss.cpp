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
    moveSpeed = 30.0f; // new
    patrolSpeed = 30.0f; // new
    savedPosX = 0.0f; // new

    //Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

    pbody->listener = this;
    //Assign collider type
    pbody->ctype = ColliderType::BOSS;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(2.0f);

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    return true;
}

bool Boss::Update(float dt)
{
    enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

    float dx = playerTilePos.getX() - enemyTilePos.getX();
    float dy = playerTilePos.getY() - enemyTilePos.getY();
    // Calculate the distance between the enemy and the player only on the X axis
    float distanceToPlayer = abs(dx);

    // Limit movement towards the player only if within X tiles
    float patrolDistance = 7.0f; // Set the maximum distance for the enemy to chase the player

    pbody->body->SetFixedRotation(false);  // Do not restrict the body's rotation
    pbody->body->SetGravityScale(1.0f);  // Adjust gravity so it doesn't excessively affect the jump

    if (!canAttack) {
        printf("%f\n", &currentAttackCooldown);
        
        currentAttackCooldown -= dt;
        if (currentAttackCooldown <= 0) {
            canAttack = true;
            currentAttackCooldown = 0.0f;
        }
    }


    if (distanceToPlayer <= attackDistance) {
        pbody->body->SetLinearVelocity(b2Vec2(0.0f, pbody->body->GetLinearVelocity().y));
    }

    isLookingLeft = dx < 0;

    

    if (distanceToPlayer <= attackDistance && canAttack && !isAttacking) {
        isAttacking = true;
        canAttack = false;
        currentAttackCooldown = attackCooldown;
        currentAnimation = &attack;
        currentAnimation->Reset();

        if (area == nullptr) {
            area = Engine::GetInstance().physics.get()->CreateRectangleSensor(
                (int)position.getX() + (isLookingLeft ? -10 : 50),
                (int)position.getY() + texH / 2,
                40,
                40,
                bodyType::KINEMATIC
            );
            area->ctype = ColliderType::BOSS_ATTACK;
        }
    }

    if (isAttacking) {
        pbody->body->SetLinearVelocity(b2Vec2(0, pbody->body->GetLinearVelocity().y));

        if (attack.HasFinished()) {
            isAttacking = false;
            currentAnimation = &running;

            if (area != nullptr) {
                Engine::GetInstance().physics.get()->DeletePhysBody(area);
                area = nullptr;
            }
        }
    }

    if (!isAttacking) {
        if (distanceToPlayer <= patrolDistance) {
            if (!resting) {
                resting = true;
                currentAnimation = &running;
                running.Reset();
            }

            if (currentAnimation == &running) {
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
        else {
            if (resting) {
                resting = false;
                currentAnimation = &idle;
                idle.Reset();
            }
            b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
            pbody->body->SetLinearVelocity(velocity);
        }
    }
    

    // change sprite direction
    if (isLookingLeft) {
        flip = SDL_FLIP_HORIZONTAL;
    }
    else {
        flip = SDL_FLIP_NONE;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);
    currentAnimation->Update();

    // pathfinding drawing
    pathfinding->DrawPath();
    pathfinding->ResetPath(enemyTilePos);

    return true;
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
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
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