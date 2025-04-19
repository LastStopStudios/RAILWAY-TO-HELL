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
    die.LoadAnimations(parameters.child("animations").child("die"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
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
    vez = 1;
	ResetPath();
    a = 0;
    kill = 1;
	return true;
}
bool Terrestre::Update(float dt)
{
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
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - 32, (int)position.getY() - 32, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);

        // When dying, don't process any other logic
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
        if (!ishurt) {
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

            if (vez < 3 && !isChasing) {
                velocityX = giro ? PATROL_SPEED : -PATROL_SPEED;
                isLookingLeft = !giro;
            }else if(!isChasing){
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
        // Apply velocity to physical body
        b2Vec2 velocity = b2Vec2(PIXEL_TO_METERS(velocityX), pbody->body->GetLinearVelocity().y);
        pbody->body->SetLinearVelocity(velocity);

        // Update position for rendering
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
    }else{ pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));/*frenar el cuerpo*/ }

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


    // Update animation
    currentAnimation->Update();

    // Draw path for debugging
    // Ensure it's drawn in chase mode regardless of direction
    if (isChasing && !pathfinding->pathTiles.empty()) {
        pathfinding->DrawPath();
    }

    return true;
}

void Terrestre::Matar() {//eliminating the enemy once dead 
    if (kill == 1) {
        kill = 2;
        Disable();//when it has to be activated use  Enable();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    }
}
bool Terrestre::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    vez = 1;
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
                pbody->body->SetAwake(false);
                // Engine::GetInstance().audio.get()->PlayFx(deathFx);
                pbody->body->SetGravityScale(0.0f);

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
                pbody->body->SetAwake(false);
                pbody->body->SetGravityScale(0.0f); // In case it's falling
            }
        }
   
        
        
    }

        break;
    case ColliderType::GIRO:
       giro = !giro;
       if(vez == 1){
           Vector2D pos = GetPosition();
           patrol1 = pos.getX();
           vez++;
       }else if (vez == 2) {
           Vector2D pos = GetPosition();
           patrol2 = pos.getX();
           vez++;
       }
        break;
	}
}

void Terrestre::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		break;
	}
}