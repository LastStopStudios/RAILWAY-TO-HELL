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


    // Inicializar textura con verificación
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
    texRadius = parameters.attribute("radius").as_int();

    // Verificar que las dimensiones sean válidas
    if (texW <= 0 || texH <= 0) {
        LOG("ERROR: Invalid spear texture dimensions: w=%d, h=%d", texW, texH);
        return false;
    }

    // Cargar animaciones con verificación
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

    // Crear cuerpo físico con verificación
    pbody = Engine::GetInstance().physics.get()->CreateCircle(
        (int)position.getX() + texH / 2,
        (int)position.getY() + texH / 2,
        texH / 2,
        bodyType::DYNAMIC
    );

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

    moveSpeed = 400.0f;

    LOG("Spear initialized successfully at position: (%f, %f)", position.getX(), position.getY());
    return true;
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

    // Si está desapareciendo, no aplicar movimiento
    if (isDisappearing) {
        if (currentAnimation->HasFinished()) {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
            return false;
        }
    }
    else {
        // Aplicar movimiento según la dirección configurada
        b2Vec2 velocity(0, 0);

        switch (spearDirection) {
        case SpearDirection::HORIZONTAL_LEFT:
            velocity.x = PIXEL_TO_METERS(-moveSpeed);
            break;
        case SpearDirection::HORIZONTAL_RIGHT:
            velocity.x = PIXEL_TO_METERS(moveSpeed);
            break;
        case SpearDirection::VERTICAL_DOWN:
            velocity.y = PIXEL_TO_METERS(moveSpeed);
            break;
        }

        if (pbody != nullptr && pbody->body != nullptr) {
            pbody->body->SetLinearVelocity(velocity);
        }
    }

    // Actualizar posición solo si pbody es válido
    if (pbody != nullptr && pbody->body != nullptr) {
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
    }

    // Configurar flip según dirección
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    double angle = 0.0;

    switch (spearDirection) {
    case SpearDirection::HORIZONTAL_LEFT:
        flip = SDL_FLIP_HORIZONTAL;
        break;
    case SpearDirection::HORIZONTAL_RIGHT:
        // Sin flip
        break;
    case SpearDirection::VERTICAL_DOWN:
        angle = 90.0;
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
void Spears::startDisappearAnimation() {
    pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    currentAnimation = &disappear;
    currentAnimation->Reset();
    isDisappearing = true;
}

bool Spears::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}

void Spears::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Spear Collision PLATFORM");
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
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Spears::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Spears::SetDirection(SpearDirection direction) {
    spearDirection = direction;
}

void Spears::SetOriginPosition(Vector2D origin) {
    originPosition = origin;
    hasCustomOrigin = true;
    position = origin;
}