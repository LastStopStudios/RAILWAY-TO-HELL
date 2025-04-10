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
    // Don't process logic if we're not in GAMEPLAY mode
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    // Handle skipping first input if necessary
    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();
        return true;
    }

    if (Engine::GetInstance().entityManager->dialogo == false) {

        // Constants to adjust enemy behavior
        const float DETECTION_DISTANCE = 200.0f;
        const float CHASE_SPEED = 50.0f;
        const float PATROL_SPEED = 30.0f;
        const int MAX_PATHFINDING_ITERATIONS = 50;

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
        isChasing = false;

        // Chase mode (high priority)
        if (distanceToPlayer <= DETECTION_DISTANCE)
        {
            isChasing = true;

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

                // Fine-tune speed when close to target
                if (fabs(moveX) < 5.0f) {
                    velocityX *= 0.5f;
                }
            }
        }

        // Patrol mode (low priority) - only if not chasing
        if (!isChasing)
        {
            velocityX = giro ? PATROL_SPEED : -PATROL_SPEED;
            isLookingLeft = !giro;
        }

        // Apply velocity to physical body
        b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(velocityX), pbody->body->GetLinearVelocity().y);
        pbody->body->SetLinearVelocity(velocity);

        // Update position for rendering
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
    }else{ pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));/*frenar el cuerpo*/ }

        // Configure sprite flip based on direction
        SDL_RendererFlip flip = isLookingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        // Draw enemy texture and animation
        Engine::GetInstance().render.get()->DrawTexture(
            texture,
            (int)position.getX(),
            (int)position.getY(),
            &currentAnimation->GetCurrentFrame(),
            1.0f, 0.0, INT_MAX, INT_MAX,
            flip
        );


    // Update animation
    currentAnimation->Update();

    // Draw path for debugging
    // Ensure it's drawn in chase mode regardless of direction
    if (isChasing && !pathfinding->pathTiles.empty()) {
        pathfinding->DrawPath();
    }

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
	case ColliderType::PLAYER_WHIP_ATTACK:
		LOG("Enemy hit by player whip attack - DESTROY");
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		break;
    case ColliderType::GIRO:
       giro = !giro;
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