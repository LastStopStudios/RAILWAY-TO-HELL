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

    // Obtener dimensiones reales de la textura
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

    // Usar las dimensiones de la textura o los valores originales como respaldo
    texW = textureWidth > 0 ? textureWidth : parameters.attribute("w").as_int();
    texH = textureHeight > 0 ? textureHeight : parameters.attribute("h").as_int();

    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texIsDeath = parameters.attribute("isDeath").as_int();

    // Calcular radio de colisi�n basado en el tama�o de la textura
    int collisionRadius = std::min(texW, texH) / 2;
    texRadius = collisionRadius;

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
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

    return true;
}

bool Volador::Update(float dt) {
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }


    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();  // Solo lo hacemos una vez
        return true;
    }
    Vector2D camPos(Engine::GetInstance().render->camera.x, Engine::GetInstance().render->camera.y);
    Vector2D camSize(Engine::GetInstance().render->camera.w, Engine::GetInstance().render->camera.h);

    // Convertir posici?n del enemigo a coordenadas de la c?mara
    Vector2D enemyScreenPos = enemyPos - camPos;

   
    Vector2D enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    float distanceToPlayerX = abs(playerTilePos.getX() - enemyTilePos.getX());
    const float patrolDistance = 7.0f;
    static bool movingRight = true;

    // Patrol and pathfinding logic
    if (distanceToPlayerX <= patrolDistance) {
        // Pathfinding and moving towards the player
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
           
        const float patrolSpeed = 20.0f;
        if (giro == true) {
            b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(patrolSpeed), 0.0f);
            pbody->body->SetLinearVelocity(velocity);
            isLookingLeft = true;
        }
        else if (giro == false) {
            b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(-patrolSpeed), 0.0f);
            pbody->body->SetLinearVelocity(velocity);
            isLookingLeft = false;
        }
        
    }

    // Flip sprite configuration
    SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // Adjust position for rendering
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Draw texture and animation
    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + 3, (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);

    currentAnimation->Update();

    pathfinding->DrawPath();
    pathfinding->ResetPath(enemyTilePos);

    return true;
}

bool Volador::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
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
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::PLAYER_ATTACK:
        LOG("Enemy hit by player attack - DESTROY");
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::GIRO:
        giro = !giro;
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

