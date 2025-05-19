
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
#include "UI.h"
#include "Bufon.h"

Bufon::Bufon() : Entity(EntityType::BUFON)
{

}

Bufon::~Bufon() {
    if (pathfinding != nullptr) {
        delete pathfinding;
        pathfinding = nullptr;
    }
}

bool Bufon::Awake() {
    return true;
}

bool Bufon::Start() {
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
    currentAnimation = &idle;

    // Add physics to the entity - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

    // Assign collider type
    pbody->ctype = ColliderType::BUFON;

    pbody->listener = this;

    if (!parameters.attribute("gravity").as_bool())
        pbody->body->SetGravityScale(0);

    // Initialize pathfinding
    pathfinding = new Pathfinding();
    ResetPath();

    // Initialize detection distance
    detectionDistance = 50.0f;  // Distance in tiles to detect player

    return true;
}

bool Bufon::Update(float dt) {
    // Update the current animation
    if (currentAnimation != nullptr) {
        currentAnimation->Update();
    }

    // Get enemy position
    enemyPos = GetPosition();
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());

    // Get player position
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    // Calculate the distance between the enemy and the player
    float dx = playerTilePos.getX() - enemyTilePos.getX();
    float dy = playerTilePos.getY() - enemyTilePos.getY();
    float distanceToPlayer = sqrt(dx * dx + dy * dy);  // Euclidean distance

    // Check if player is within detection range
    if (distanceToPlayer <= detectionDistance) {
        LOG("HOLI");
        
    }

    // Always reset the path before propagating
    pathfinding->ResetPath(enemyTilePos);

    // Always propagate A* algorithm to calculate path to player
    int maxIterations = 100;
    int iterations = 0;

    // Generate path to player using A* algorithm
    while (pathfinding->pathTiles.empty() && iterations < maxIterations) {
        pathfinding->PropagateAStar(SQUARED);
        iterations++;
    }

    // Draw the path (after it has been calculated)
    pathfinding->DrawPath();

    // Update sprite position based on the physical body position
    position.setX(METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - texW / 2);
    position.setY(METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - texH / 2);

    Draw();

    return true;
}

bool Bufon::CleanUp() {
    // Release resources
    if (pathfinding != nullptr) {
        delete pathfinding;
        pathfinding = nullptr;
    }
    return true;
}

void Bufon::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Bufon::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Bufon::ResetPath() {
    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
    pathfinding->ResetPath(tilePos);
}

void Bufon::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        // Logic when colliding with the player
        break;
    }
}

void Bufon::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
        // Logic when the collision with the player ends
        break;
    }
}

// Helper methods for interacting with Animation
int Bufon::GetCurrentFrameId() const {
    if (currentAnimation != nullptr) {
        // Use the GetCurrentFrameIndex method from Animation
        return currentAnimation->GetCurrentFrameIndex();
    }
    return 0;
}

int Bufon::GetTotalFrames() const {
    if (currentAnimation != nullptr) {
        // Access totalFrames from Animation
        return currentAnimation->totalFrames;
    }
    return 0;
}

void Bufon::Draw() {
    if (texture != nullptr && currentAnimation != nullptr) {
        // Get the current animation frame (source section)
        SDL_Rect rect = currentAnimation->GetCurrentFrame();

        // Render the texture with the current animation
        // Parameters: texture, posX, posY, section, speed, angle, pivotX, pivotY, flip
        Engine::GetInstance().render.get()->DrawTexture(
            texture,                 // texture
            (int)position.getX(),    // position X
            (int)position.getY(),    // position Y
            &rect,                   // source section (SDL_Rect*)
            1.0f,                    // speed (1.0 = normal speed)
            0.0,                     // rotation angle (0 = no rotation)
            INT_MAX,                 // pivotX (INT_MAX = no pivot)
            INT_MAX,                 // pivotY (INT_MAX = no pivot)
            SDL_FLIP_NONE            // no flipping
        );
    }
}