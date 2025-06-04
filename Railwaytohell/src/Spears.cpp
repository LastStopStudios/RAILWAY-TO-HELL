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
    maxLifeTime = 3000.0f;  // 4 segundos
    useLifeTimer = false;
    platformCollisionCount = 0;
    maxPlatformCollisions = 2;  // Se borra en la segunda colision

    // Velocidades diferentes para cada dirección
    horizontalSpeed = 600.0f;  // Velocidad para lanzas horizontales
    verticalSpeed = 1500.0f;    // Velocidad para lanzas verticales (más rápida)
    moveSpeed = 600.0f;        // Velocidad por defecto

    // Inicializar variables de sombra
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

    // Si no hay posición personalizada, usar la del XML
    if (!hasCustomOrigin) {
        position.setX(parameters.attribute("x").as_int());
        position.setY(parameters.attribute("y").as_int());
    }
    else {
        position = originPosition;
    }

    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Cargar animaciones 
    pugi::xml_node fallingNode = parameters.child("animations").child("falling");
    pugi::xml_node disappearNode = parameters.child("animations").child("disappear");

    if (!fallingNode || !disappearNode) {
        LOG("ERROR: Missing animation nodes in spear configuration");
        return false;
    }

    falling.LoadAnimations(fallingNode);
    disappear.LoadAnimations(disappearNode);

    currentAnimation = &falling;

    // Verificar que la animación se haya cargado correctamente
    if (currentAnimation == nullptr) {
        LOG("ERROR: Failed to set current animation for spear");
        return false;
    }

    // Crear cuerpo físico según la dirección
    CreatePhysicsBody();

    if (pbody == nullptr) {
        LOG("ERROR: Failed to create physics body for spear");
        return false;
    }

    // Configurar tipo de colisionador
    pbody->ctype = ColliderType::SPEAR;
    pbody->listener = this;

    // Configurar gravedad si es necesario
    if (!parameters.attribute("gravity").as_bool()) {
        pbody->body->SetGravityScale(0);
    }

    // Configurar velocidades específicas según la dirección
    switch (spearDirection) {
    case SpearDirection::HORIZONTAL_LEFT:
    case SpearDirection::HORIZONTAL_RIGHT:
        moveSpeed = horizontalSpeed;
        useLifeTimer = true;
        lifeTimer = 0.0f;
        platformCollisionCount = 0;
        LOG("Horizontal spear configured with speed: %f", moveSpeed);
        break;
    case SpearDirection::VERTICAL_DOWN:
        moveSpeed = verticalSpeed;
        SetupShadow();
        LOG("Vertical spear configured with speed: %f", moveSpeed);
        break;
    }

    LOG("Spear initialized successfully at position: (%f, %f) with speed: %f",
        position.getX(), position.getY(), moveSpeed);
    return true;
}

void Spears::SetupShadow() {
    // Cargar textura de sombra para lanzas verticales
    shadowTexture = Engine::GetInstance().textures.get()->Load("Assets/Textures/bosses/SpearShadow.png");

    if (shadowTexture != nullptr) {
        hasShadow = true;

        shadowGroundY = FindGroundLevel();
        LOG("Shadow created for vertical spear at ground level: %f", shadowGroundY);
    }
    else {
        LOG("WARNING: Could not load shadow texture for vertical spear");
        hasShadow = false;
    }
}

float Spears::FindGroundLevel() {
    return position.getY() + 2320.0f; // Asumiendo que el suelo está 600 pixels abajo
}

void Spears::CreatePhysicsBody() {
    int bodyWidth, bodyHeight;
    bool isSensor = false;

    // Ajustar dimensiones del colisionador según la dirección
    switch (spearDirection) {
    case SpearDirection::VERTICAL_DOWN:
        // Para movimiento vertical, la lanza es más alta que ancha
        bodyWidth = texW;  // Más estrecha horizontalmente
        bodyHeight = texH + 200;     // Mantiene altura completa
        isSensor = false; // Colisión física normal
        break;
    case SpearDirection::HORIZONTAL_LEFT:
    case SpearDirection::HORIZONTAL_RIGHT:
    default:
        // Para movimiento horizontal, la lanza es más ancha que alta
        bodyWidth = texW + 200;      // Mantiene ancho completo
        bodyHeight = texH / 4; // Más baja verticalmente
        isSensor = true; // Crear como sensor
        break;
    }

    // Crear el cuerpo físico directamente como sensor si es necesario
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
        LOG("Physics body created successfully for spear at (%f, %f) - Sensor: %s",
            position.getX(), position.getY(), isSensor ? "true" : "false");
    }
    else {
        LOG("ERROR: Failed to create physics body for spear");
    }
}

bool Spears::Update(float dt)
{
    // Verificar que la textura y animación estén válidas antes de renderizar
    if (texture == nullptr) {
        LOG("ERROR: Spear texture is null!");
        return false;
    }

    if (currentAnimation == nullptr) {
        LOG("ERROR: Spear currentAnimation is null!");
        return false;
    }

    // Verificar timer de vida para lanzas horizontales
    if (useLifeTimer && !isDisappearing) {
        lifeTimer += dt;
        if (lifeTimer >= maxLifeTime) {
            LOG("Horizontal spear life timer expired, starting disappear animation");
            startDisappearAnimation();
        }
    }

    // Si está desapareciendo, no aplicar movimiento
    if (isDisappearing) {
        if (currentAnimation->HasFinished()) {
            // Limpiar el listener antes de marcar para eliminación
            if (pbody != nullptr) {
                pbody->listener = nullptr;
            }
            pendingToDelete = true;
            return false;
        }
    }
    else {
        // Aplicar movimiento según la dirección configurada usando moveSpeed
        b2Vec2 velocity(0, 0);

        switch (spearDirection) {
        case SpearDirection::HORIZONTAL_LEFT:
            velocity.x = PIXEL_TO_METERS(-moveSpeed); // Usa horizontalSpeed
            break;
        case SpearDirection::HORIZONTAL_RIGHT:
            velocity.x = PIXEL_TO_METERS(moveSpeed);  // Usa horizontalSpeed
            break;
        case SpearDirection::VERTICAL_DOWN:
            velocity.y = PIXEL_TO_METERS(moveSpeed);  // Usa verticalSpeed
            break;
        }

        // Verificar que el cuerpo físico sea válido antes de aplicar velocidad
        if (pbody != nullptr && pbody->body != nullptr && !pendingToDelete && !isDisappearing) {
            try {
                pbody->body->SetLinearVelocity(velocity);
            }
            catch (...) {
                LOG("Warning: Exception setting spear velocity");
            }
        }
    }

    // Actualizar posición solo si pbody es válido
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

    // Renderizar sombra si es una lanza vertical y tiene sombra
    if (hasShadow && spearDirection == SpearDirection::VERTICAL_DOWN && shadowTexture != nullptr && !isDisappearing) {
        RenderShadow();
    }

    // Configurar flip y rotación según dirección
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    double angle = 0.0;

    switch (spearDirection) {
    case SpearDirection::HORIZONTAL_LEFT:
        flip = SDL_FLIP_HORIZONTAL;
        angle = -90.0;  // Rotar 90° para orientación horizontal
        break;
    case SpearDirection::HORIZONTAL_RIGHT:
        flip = SDL_FLIP_NONE;
        angle = 90.0;  // Rotar 90° para orientación horizontal
        break;
    case SpearDirection::VERTICAL_DOWN:
        flip = SDL_FLIP_NONE;
        angle = 180.0;  // Rotar 180° para que mire hacia abajo
        break;
    }

    // Obtener el frame actual y verificar que sea válido
    SDL_Rect currentFrame = currentAnimation->GetCurrentFrame();

    // Verificar que el frame tenga dimensiones válidas
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

    // Calcular la posición X de la sombra (misma X que la lanza)
    float shadowX = position.getX();

    // La sombra siempre está en el nivel del suelo
    float shadowY = shadowGroundY - 32; // Ajustar según el tamaño de tu textura de sombra

    // Renderizar la sombra con menor opacidad
    Engine::GetInstance().render.get()->DrawTexture(
        shadowTexture,
        (int)shadowX + 10,
        (int)shadowY,
        nullptr, // Usar toda la textura
        1.0f,    // Escala normal
        0.0,     // Sin rotación
        128,     // Opacidad reducida (0-255)
        128,     // Opacidad reducida
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

    // Eliminar la sombra cuando la lanza comience a desaparecer
    if (hasShadow) {
        hasShadow = false;
        LOG("Shadow removed for disappearing spear");
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

    // Limpiar sombra si existe
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
        // Lógica especial para lanzas horizontales (sensores)
        LOG("Spear sensor detected collision with %d", physB->ctype);

        // Solo contar colisiones con plataformas
        if (physB->ctype == ColliderType::PLATFORM) {
            if (veces == 2) {
                startDisappearAnimation();
            }
        }
        return;
    }

    // Lógica original para lanzas verticales
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Spear Collision PLATFORM");
        // Eliminar sombra al tocar el suelo
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

    // Solo actualizar el cuerpo físico si existe
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

    // Configurar velocidad y timer según la nueva dirección
    if (direction == SpearDirection::HORIZONTAL_LEFT ||
        direction == SpearDirection::HORIZONTAL_RIGHT) {
        moveSpeed = horizontalSpeed;  // Usar velocidad horizontal
        useLifeTimer = true;
        lifeTimer = 0.0f;
        platformCollisionCount = 0;

        // Eliminar sombra para lanzas horizontales
        if (hasShadow) {
            hasShadow = false;
        }
        LOG("Direction changed to horizontal, speed set to: %f", moveSpeed);
    }
    else if (direction == SpearDirection::VERTICAL_DOWN) {
        moveSpeed = verticalSpeed;     // Usar velocidad vertical
        useLifeTimer = false;
        lifeTimer = 0.0f;
        platformCollisionCount = 0;

        // Configurar sombra para lanzas verticales
        SetupShadow();
        LOG("Direction changed to vertical, speed set to: %f", moveSpeed);
    }

    // Si ya existe un cuerpo físico, recrearlo con las nuevas dimensiones
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

// Métodos adicionales para configurar velocidades dinámicamente
void Spears::SetHorizontalSpeed(float speed) {
    horizontalSpeed = speed;
    // Si la lanza actual es horizontal, actualizar moveSpeed inmediatamente
    if (spearDirection == SpearDirection::HORIZONTAL_LEFT ||
        spearDirection == SpearDirection::HORIZONTAL_RIGHT) {
        moveSpeed = horizontalSpeed;
        LOG("Horizontal speed updated to: %f", horizontalSpeed);
    }
}

void Spears::SetVerticalSpeed(float speed) {
    verticalSpeed = speed;
    // Si la lanza actual es vertical, actualizar moveSpeed inmediatamente
    if (spearDirection == SpearDirection::VERTICAL_DOWN) {
        moveSpeed = verticalSpeed;
        LOG("Vertical speed updated to: %f", verticalSpeed);
    }
}

float Spears::GetHorizontalSpeed() const {
    return horizontalSpeed;
}

float Spears::GetVerticalSpeed() const {
    return verticalSpeed;
}