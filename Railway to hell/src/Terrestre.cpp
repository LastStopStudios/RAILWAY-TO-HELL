#include "Terrestre.h"
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

Terrestre::Terrestre() : Entity(EntityType::TERRESTRE)
{

}

Terrestre::~Terrestre() {
	delete pathfinding;
}

bool Terrestre::Awake() {
	return true;
}

bool Terrestre::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnimation = &idle;
	
	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	//Assign collider type
	pbody->ctype = ColliderType::TERRESTRE;

	pbody->listener = this;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Initialize pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	return true;
}

bool Terrestre::Update(float dt)
{
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();  
        return true;
    }
    // Get the enemy's current position
    enemyPos = GetPosition();

    // Get the player's position
    Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();

    // Convert world coordinates to tile coordinates
    Vector2D enemyTilePos = Engine::GetInstance().map.get()->WorldToMap(enemyPos.getX(), enemyPos.getY());
    Vector2D playerTilePos = Engine::GetInstance().map.get()->WorldToMap(playerPos.getX(), playerPos.getY());

    // Calculate the distance to the player
    float distanceToPlayer = abs(playerPos.getX() - enemyPos.getX());

    // Player detection distance (in pixels)
    float detectionDistance = 200.0f;
    float moveSpeed = 50.0f;

    // If the player is within detection range
    if (distanceToPlayer <= detectionDistance) {
        // Reset the path with the enemy's current position
        pathfinding->ResetPath(enemyTilePos);

        // Run A* pathfinding to find the path to the player
        // Only propagate A* a limited number of times
        for (int i = 0; i < 50; i++) {
            pathfinding->PropagateAStar(SQUARED);

            // Stop propagating if we already found a path to the player
            if (pathfinding->ReachedPlayer(playerTilePos)) {
                break;
            }
        }

        // If a valid path is found with at least 2 points
        if (pathfinding->pathTiles.size() >= 2) {
            // Get the next point in the path (second to last)
            auto it = pathfinding->pathTiles.end();
            --it; // Last element

            // Ensure there is at least one more point
            if (it != pathfinding->pathTiles.begin()) {
                --it; // Second to last element
                Vector2D nextTile = *it;

                // Convert tile position to world coordinates
                Vector2D nextPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());

                // Calculate movement direction towards the next point
                float dx = nextPos.getX() - enemyPos.getX();

                // Determine movement direction
                if (dx < 0) {
                    isLookingLeft = true;
                }
                else {
                    isLookingLeft = false;
                }

                // Calculate movement speed based on direction
                float stepX = isLookingLeft ? -moveSpeed : moveSpeed;

                // Apply velocity to the physics body
                b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(stepX), pbody->body->GetLinearVelocity().y);
                pbody->body->SetLinearVelocity(velocity);
            }
        }
    }
    else {
        // If the player is out of range, patrol
        const float patrolSpeed = 30.0f;
        if (giro == true) {
            b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(patrolSpeed), pbody->body->GetLinearVelocity().y);
            pbody->body->SetLinearVelocity(velocity);
            isLookingLeft = false;
        }
        else if (giro == false) {
            b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(-patrolSpeed), pbody->body->GetLinearVelocity().y);
            pbody->body->SetLinearVelocity(velocity);
            isLookingLeft = true;
        }
       
       
    }

    // Set sprite flip based on direction
    SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // Update position for rendering
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);

    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Draw the enemy texture and animation
    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);
    currentAnimation->Update();

    // Draw the path for debugging
    pathfinding->DrawPath();

    return true;
}

bool Terrestre::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	return true;
}

void Terrestre::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Terrestre::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Terrestre::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Terrestre::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with player - DESTROY");
		//Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
    case ColliderType::PLAYER_ATTACK:
        LOG("Enemy hit by player attack - DESTROY");
        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::GIRO:
       giro = !giro;
        LOG("Toco");
        break;
	}
}

void Terrestre::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		break;
	}
}