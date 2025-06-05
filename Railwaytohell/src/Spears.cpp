#include "Spears.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Spears::Spears() : Entity(EntityType::SPEAR)
{
    lifeTimer = 0.0f;
    maxLifeTime = 3000.0f;  // 4 seconds
    useLifeTimer = false;
    platformCollisionCount = 0;
	maxPlatformCollisions = 2;  // Delete in the second collision

	// Diferent speeds for horizontal and vertical spears
    horizontalSpeed = 600.0f;  
    verticalSpeed = 1500.0f;   
    moveSpeed = 600.0f;        // normal speed

	// Initialize shadow properties
    shadowTexture = nullptr;
    hasShadow = false;
    shadowGroundY = 0.0f;
}

Spears::~Spears() {}

bool Spears::Awake() {
    return true;
}

bool Spears::Start() {
    if (!parameters) {
        LOG("ERROR: Spears parameters node is null!");
        return false;
    }

    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());

	// If theres no cutom coordinates, use the default ones
    if (!hasCustomOrigin) {
        position.setX(parameters.attribute("x").as_int());
        position.setY(parameters.attribute("y").as_int());
    }
    else {
        position = originPosition;
    }

    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

	// Load animations
    pugi::xml_node fallingNode = parameters.child("animations").child("falling");
    pugi::xml_node disappearNode = parameters.child("animations").child("disappear");

    if (!fallingNode || !disappearNode) {
        LOG("ERROR: Missing animation nodes in spear configuration");
        return false;
    }

    falling.LoadAnimations(fallingNode);
    disappear.LoadAnimations(disappearNode);

    currentAnimation = &falling;

    if (currentAnimation == nullptr) {
        LOG("ERROR: Failed to set current animation for spear");
        return false;
    }

    CreatePhysicsBody();

    if (pbody == nullptr) {
        LOG("ERROR: Failed to create physics body for spear");
        return false;
    }

    pbody->ctype = ColliderType::SPEAR;
    pbody->listener = this;

    if (!parameters.attribute("gravity").as_bool()) {
        pbody->body->SetGravityScale(0);
    }

    switch (spearDirection) {
    case SpearDirection::HORIZONTAL_LEFT:
    case SpearDirection::HORIZONTAL_RIGHT:
        moveSpeed = horizontalSpeed;
        useLifeTimer = true;
        lifeTimer = 0.0f;
        platformCollisionCount = 0;
        break;
    case SpearDirection::VERTICAL_DOWN:
        moveSpeed = verticalSpeed;
        SetupShadow();
        break;
    }

        position.getX(), (position.getY(), moveSpeed);
    return true;
}

void Spears::SetupShadow() {
	// Load the shadow texture for vertical spears
    shadowTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/bosses/SpearShadow.png");

    if (shadowTexture != nullptr) {
        hasShadow = true;

        shadowGroundY = FindGroundLevel();
    }
    else {
        hasShadow = false;
    }
}

float Spears::FindGroundLevel() {
    return position.getY() + 2110.0f; 
}

void Spears::CreatePhysicsBody() {
    int bodyWidth, bodyHeight;
    bool isSensor = false;

    // Adjust collision dimensions according to direction
    switch (spearDirection) {
    case SpearDirection::VERTICAL_DOWN:
        // For vertical movement, the spear is taller than it is wide.
        bodyWidth = texW;  // Narrower horizontally
        bodyHeight = texH + 200; // Maintains full height
        isSensor = false; // Normal physical collision
        break;
    case SpearDirection::HORIZONTAL_LEFT:
    case SpearDirection::HORIZONTAL_RIGHT:
    default:
        // For horizontal movement, the spear is wider than it is tall.
        bodyWidth = texW + 200;      // Maintains full width
        bodyHeight = texH / 4; // Lower vertically
        isSensor = true; // Create as a sensor
        break;
    }

    //Create the physical body directly as a sensor if necessary.
    if (isSensor) {
        pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
            (int)position.getX() + texW / 2,
            (int)position.getY() + texH / 2,
            bodyWidth / 2,
            bodyHeight / 2,
            bodyType::DYNAMIC
        );
        isSensorBody = true;
    }
    else {
        pbody = Engine::GetInstance().physics.get()->CreateRectangle(
            (int)position.getX() + texW / 2,
            (int)position.getY() + texH / 2,
            bodyWidth / 2,
            bodyHeight / 2,
            bodyType::DYNAMIC
        );
        isSensorBody = false;
    }

    if (pbody != nullptr) {
            position.getX(), (position.getY(), isSensor ? "true" : "false");
    }
    else {
        LOG("ERROR: Failed to create physics body for spear");
    }
}

bool Spears::Update(float dt)
{
    //Verify that the texture and animation are valid before rendering.
    if (texture == nullptr) {
        LOG("ERROR: Spear texture is null!");
        return false;
    }

    if (currentAnimation == nullptr) {
        LOG("ERROR: Spear currentAnimation is null!");
        return false;
    }

    //Check life timer for horizontal lances
    if (useLifeTimer && !isDisappearing) {
        lifeTimer += dt;
        if (lifeTimer >= maxLifeTime) {
            startDisappearAnimation();
        }
    }

    // If it is disappearing, do not apply movement.
    if (isDisappearing) {
        if (currentAnimation->HasFinished()) {
            // Clear the listener before marking for deletion
            if (pbody != nullptr) {
                pbody->listener = nullptr;
            }
            pendingToDelete = true;
            return false;
        }
    }
    else {
        // Apply movement according to the configured direction using moveSpeed
        b2Vec2 velocity(0, 0);

        switch (spearDirection) {
        case SpearDirection::HORIZONTAL_LEFT:
            velocity.x = PIXEL_TO_METERS(-moveSpeed); // Use horizontalSpeed
            break;
        case SpearDirection::HORIZONTAL_RIGHT:
            velocity.x = PIXEL_TO_METERS(moveSpeed);  // Use horizontalSpeed
            break;
        case SpearDirection::VERTICAL_DOWN:
            velocity.y = PIXEL_TO_METERS(moveSpeed);  // Use verticalSpeed
            break;
        }

        // Verify that the physical body is valid before applying speed
        if (pbody != nullptr && pbody->body != nullptr && !pendingToDelete && !isDisappearing) {
            try {
                pbody->body->SetLinearVelocity(velocity);
            }
            catch (...) {
                LOG("Warning: Exception setting spear velocity");
            }
        }
    }

    // Update position only if pbody is valid
    if (pbody != nullptr && pbody->body != nullptr && !pendingToDelete && !isDisappearing) {
        try {
            b2Transform pbodyPos = pbody->body->GetTransform();
            position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
            position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
        }
        catch (...) {
            LOG("Warning: Exception updating spear position from physics body");
        }
    }

    // Render shadow if it is a vertical spear and has a shadow
    if (hasShadow && spearDirection == SpearDirection::VERTICAL_DOWN && shadowTexture != nullptr && !isDisappearing) {
        RenderShadow();
    }

    // Configure flip and rotation according to direction
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    double angle = 0.0;

    switch (spearDirection) {
    case SpearDirection::HORIZONTAL_LEFT:
        flip = SDL_FLIP_HORIZONTAL;
        angle = -90.0;  // Rotate 90° for horizontal orientation
        break;
    case SpearDirection::HORIZONTAL_RIGHT:
        flip = SDL_FLIP_NONE;
        angle = 90.0;  // Rotate 90° for horizontal orientation
        break;
    case SpearDirection::VERTICAL_DOWN:
        flip = SDL_FLIP_NONE;
        angle = 180.0;  // Rotate 180° so that it faces downwards.
        break;
    }

    // Get the current frame and verify that it is valid
    SDL_Rect currentFrame = currentAnimation->GetCurrentFrame();

    // Verify that the frame has valid dimensions.
    if (currentFrame.w <= 0 || currentFrame.h <= 0) {
        LOG("ERROR: Invalid animation frame dimensions: w=%d, h=%d", currentFrame.w, currentFrame.h);
        return false;
    }

    // Renderizar solo si todo es válido
    bool renderResult = Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX(),
        (int)position.getY(),
        &currentFrame,
        1.0f,
        angle,
        INT_MAX,
        INT_MAX,
        flip
    );

    if (!renderResult) {
        LOG("ERROR: Failed to render spear texture");
    }

    currentAnimation->Update();

    return true;
}

void Spears::RenderShadow() {
    if (!hasShadow || shadowTexture == nullptr) {
        return;
    }

	// Calcula the spear's shadow position based on its current position
    float shadowX = position.getX();

    // Shadow Y is always at the ground level
    float shadowY = shadowGroundY - 32; 

	// Render shadow with less opacity
    Engine::GetInstance().render.get()->DrawTexture(
        shadowTexture,
        (int)shadowX + 10,
        (int)shadowY,
        nullptr, 
        1.0f,    
        0.0,     
        128,    
        128,    
        SDL_FLIP_NONE
    );
}

void Spears::startDisappearAnimation() {
    if (pbody != nullptr && pbody->body != nullptr && !pendingToDelete) {
        try {
            pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        }
        catch (...) {
            LOG("Warning: Exception stopping spear movement in disappear animation");
        }
    }

    currentAnimation = &disappear;
    currentAnimation->Reset();
    isDisappearing = true;

	// Delete shadow when the spear starts disappearing
    if (hasShadow) {
        hasShadow = false;
    }
}

bool Spears::CleanUp() {
    LOG("Cleaning up spear");

    // Clear the listener first to prevent callbacks during deletion
    if (pbody != nullptr) {
        pbody->listener = nullptr;

        // Delete the physics body
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
        pbody = nullptr;
    }

	// Clear shadow texture if it exists
    if (hasShadow) {
        hasShadow = false;
        shadowTexture = nullptr; 
    }

    // Mark as pending deletion 
    pendingToDelete = true;

    return true;
}

void Spears::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (isSensorBody) {
        // Horizontal spears logic
        LOG("Spear sensor detected collision with %d", physB->ctype);

        if (physB->ctype == ColliderType::PLATFORM) {
            if (veces == 2) {
                startDisappearAnimation();
            }
        }
        return;
    }

	// Vertical spears logic
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Spear Collision PLATFORM");
		// Delete shadow when hitting the ground
        if (hasShadow && spearDirection == SpearDirection::VERTICAL_DOWN) {
            hasShadow = false;
            LOG("Shadow removed - spear hit ground");
        }
        startDisappearAnimation();
        break;
    case ColliderType::DEVIL:
        LOG("Spear Collision DEVIL");
        startDisappearAnimation();
        break;
    case ColliderType::ITEM:
        LOG("Spear Collision ITEM");
        startDisappearAnimation();
        break;
    case ColliderType::TERRESTRE:
        LOG("Spear Collision TERRESTRE");
        startDisappearAnimation();
        break;
    case ColliderType::VOLADOR:
        LOG("Spear Collision VOLADOR");
        startDisappearAnimation();
        break;
    case ColliderType::CARONTE:
        LOG("Spear Collision CARONTE");
        startDisappearAnimation();
        break;
    case ColliderType::BOSS:
        LOG("Spear Collision BOSS");
        startDisappearAnimation();
        break;
    case ColliderType::PLAYER:
        LOG("Spear Collision PLAYER");
        startDisappearAnimation();
        break;
    case ColliderType::UNKNOWN:
        LOG("Spear Collision UNKNOWN");
        startDisappearAnimation();
        break;
    default:
        break;
    }
}

void Spears::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Spear End Collision PLATFORM");
        veces++;
        break;
    case ColliderType::ITEM:
        LOG("Spear End Collision ITEM");
        break;
    case ColliderType::ENEMY:
        LOG("Spear End Collision ENEMY");
        break;
    case ColliderType::VOLADOR:
        LOG("Spear End Collision VOLADOR");
        break;
    case ColliderType::UNKNOWN:
        LOG("Spear End Collision UNKNOWN");
        break;
    default:
        break;
    }
}

void Spears::SetPosition(Vector2D pos) {
    position = pos;

    if (pbody != nullptr && pbody->body != nullptr) {
        pos.setX(pos.getX() + texW / 2);
        pos.setY(pos.getY() + texH / 2);
        b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
        pbody->body->SetTransform(bodyPos, 0);
    }
}

Vector2D Spears::GetPosition() {
    // First check if the object is marked for deletion or disappearing
    if (pendingToDelete || isDisappearing) {
        return position; // Return cached logical position
    }

    //  body validation
    if (pbody != nullptr && pbody->body != nullptr) {
        try {
            b2Body* body = pbody->body;

            // Check if the body's world is still valid
            b2World* world = body->GetWorld();
            if (world == nullptr) {
                LOG("Warning: Physics body world is null, using logical position");
                return position;
            }

            // Get the transform safely
            b2Transform bodyTransform = body->GetTransform();
            b2Vec2 bodyPos = bodyTransform.p;

            // Update our cached position before returning
            position.setX(METERS_TO_PIXELS(bodyPos.x) - texW / 2);
            position.setY(METERS_TO_PIXELS(bodyPos.y) - texH / 2);

            return Vector2D(METERS_TO_PIXELS(bodyPos.x) - texW / 2,
                METERS_TO_PIXELS(bodyPos.y) - texH / 2);
        }
        catch (const std::exception& e) {
            LOG("Exception in GetPosition(): %s - Using logical position", e.what());
            return position;
        }
        catch (...) {
            LOG("Unknown exception in GetPosition() - Using logical position");
            return position;
        }
    }

    // If no valid physics body, return logical position
    return position;
}

void Spears::SetDirection(SpearDirection direction) {
    spearDirection = direction;

    // Set speed and timer according to the new direction
    if (direction == SpearDirection::HORIZONTAL_LEFT ||
        direction == SpearDirection::HORIZONTAL_RIGHT) {
        moveSpeed = horizontalSpeed;  // Use horizontal speed
        useLifeTimer = true;
        lifeTimer = 0.0f;
        platformCollisionCount = 0;

        // Remove shadow for horizontal spears
        if (hasShadow) {
            hasShadow = false;
        }
        LOG("Direction changed to horizontal, speed set to: %f", moveSpeed);
    }
    else if (direction == SpearDirection::VERTICAL_DOWN) {
        moveSpeed = verticalSpeed;     // Use vertical speed
        useLifeTimer = false;
        lifeTimer = 0.0f;
        platformCollisionCount = 0;

        // Set up shadow for vertical spears
        SetupShadow();
        LOG("Direction changed to vertical, speed set to: %f", moveSpeed);
    }

    // If a physics body already exists, recreate it with the new dimensions
    if (pbody != nullptr) {
        LOG("Recreating physics body for direction change");
        Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
        pbody = nullptr;

        CreatePhysicsBody();

        if (pbody != nullptr) {
            pbody->ctype = ColliderType::SPEAR;
            pbody->listener = this;

            if (parameters && !parameters.attribute("gravity").as_bool()) {
                pbody->body->SetGravityScale(0);
            }
        }
    }
}


void Spears::SetOriginPosition(Vector2D origin) {
    originPosition = origin;
    hasCustomOrigin = true;
    position = origin;
}

// Additional methods to configure speeds dynamically
void Spears::SetHorizontalSpeed(float speed) {
    horizontalSpeed = speed;
    // If the current spear is horizontal, update moveSpeed immediately
    if (spearDirection == SpearDirection::HORIZONTAL_LEFT ||
        spearDirection == SpearDirection::HORIZONTAL_RIGHT) {
        moveSpeed = horizontalSpeed;
    }
}

void Spears::SetVerticalSpeed(float speed) {
    verticalSpeed = speed;
    // If the current spear is vertical, update moveSpeed immediately
    if (spearDirection == SpearDirection::VERTICAL_DOWN) {
        moveSpeed = verticalSpeed;
    }
}

float Spears::GetHorizontalSpeed() const {
    return horizontalSpeed;
}

float Spears::GetVerticalSpeed() const {
    return verticalSpeed;
}