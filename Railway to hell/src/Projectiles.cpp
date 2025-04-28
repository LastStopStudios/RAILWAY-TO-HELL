#include "Projectiles.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Projectiles::Projectiles() : Entity(EntityType::PROJECTILE)
{

}

Projectiles::~Projectiles() {}

bool Projectiles::Awake() {
    return true;
}

bool Projectiles::Start() {

    //initilize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    texRadius = parameters.attribute("radius").as_int();

    projectileType = parameters.attribute("type").as_string();

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    if (projectileType == "arrow") {
        impact.LoadAnimations(parameters.child("animations").child("impact"));
    }
    currentAnimation = &idle;

    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texRadius / 2, bodyType::DYNAMIC);

    pbody->ctype = ColliderType::PROJECTILE;

    pbody->listener = this;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

    moveSpeed = 200.5f;

    return true;
}

bool Projectiles::Update(float dt)
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

    if (isImpacting && projectileType == "arrow") {
        currentAnimation->Update();

        if (currentAnimation->HasFinished()) {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
            return true;
        }

        if (isLookingLeft && !isLookingUp) {
            Engine::GetInstance().render.get()->DrawTexture(
                texture,
                (int)position.getX() - 5,
                (int)position.getY(),
                &currentAnimation->GetCurrentFrame(),
                1.0f,
                0.0f,
                INT_MAX,
                INT_MAX,
                SDL_FLIP_HORIZONTAL
            );
        }
        else if (!isLookingLeft && !isLookingUp) {
            Engine::GetInstance().render.get()->DrawTexture(
                texture,
                (int)position.getX() + 15,
                (int)position.getY(),
                &currentAnimation->GetCurrentFrame(),
                1.0f,
                0.0f,
                INT_MAX,
                INT_MAX,
                SDL_FLIP_NONE
            );
        }

        return true;
    }

    float angle = 0.0f;
    if (isLookingLeft && !isLookingUp) {
        pbody->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(-moveSpeed), pbody->body->GetLinearVelocity().y));
    }
    else if (!isLookingLeft && !isLookingUp) pbody->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(moveSpeed), pbody->body->GetLinearVelocity().y));

    if (isLookingUp) {
        pbody->body->SetLinearVelocity(b2Vec2(pbody->body->GetLinearVelocity().x, PIXEL_TO_METERS(-moveSpeed)));
        angle = 270.0f;
    }
    else angle = 0.0f;

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (isLookingLeft) {
        flip = SDL_FLIP_HORIZONTAL;
    }
    if (projectileType == "arrow") {
        if (isLookingLeft && !isLookingUp) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - 5, (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, angle, INT_MAX, INT_MAX, flip);
        if (!isLookingLeft && !isLookingUp) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + 15, (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, angle, INT_MAX, INT_MAX, flip);
    }
    else
    {
        if (isLookingLeft && !isLookingUp) Engine::GetInstance().render.get()->DrawTextureScaled(texture, (int)position.getX() - 5, (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, angle, INT_MAX, INT_MAX, flip);
        if (!isLookingLeft && !isLookingUp) Engine::GetInstance().render.get()->DrawTextureScaled(texture, (int)position.getX() - 5, (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, angle, INT_MAX, INT_MAX, flip);
    }
    if (isLookingUp) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, angle, INT_MAX, INT_MAX, flip);

    currentAnimation->Update();

    return true;
}

bool Projectiles::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}


void Projectiles::OnCollision(PhysBody* physA, PhysBody* physB) {
    if (isImpacting) return;

    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Collision PLATFORM");
        if (projectileType == "arrow") {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            currentAnimation = &impact;
            currentAnimation->Reset();
            isImpacting = true;
        }
        else {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        }
        break;

    case ColliderType::ITEM:
        LOG("Collision ITEM");
        if (projectileType == "arrow") {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            currentAnimation = &impact;
            currentAnimation->Reset();
            isImpacting = true;
        }
        else {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        }
        break;

    case ColliderType::TERRESTRE:
        LOG("Collision Terrestre");
        if (projectileType == "arrow") {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            currentAnimation = &impact;
            currentAnimation->Reset();
            isImpacting = true;
        }
        else {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        }
        break;

    case ColliderType::VOLADOR:
        LOG("Collision volador");
        if (projectileType == "arrow") {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            currentAnimation = &impact;
            currentAnimation->Reset();
            isImpacting = true;
        }
        else {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        }
        break;

    case ColliderType::BOSS:
        LOG("Collision BOSS");
        if (projectileType != "arrow") {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        }
        break;

    case ColliderType::UNKNOWN:
        LOG("Collision UNKNOWN");
        if (projectileType == "arrow") {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            currentAnimation = &impact;
            currentAnimation->Reset();
            isImpacting = true;
        }
        else {
            Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        }
        break;

    case ColliderType::PLAYER:
        LOG("Collision PLAYER");
        if (projectileType == "arrow") {
            pbody->body->SetLinearVelocity(b2Vec2(0, 0));
            currentAnimation = &impact;
            currentAnimation->Reset();
            isImpacting = true;
        }
        break;
    default:
        break;
    }
}

void Projectiles::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("End Collision PLATFORM");
        break;
    case ColliderType::ITEM:
        LOG("End Collision ITEM");
        break;
    case ColliderType::UNKNOWN:
        LOG("End Collision UNKNOWN");
        break;
    default:
        break;
    }
}

void Projectiles::SetPosition(Vector2D pos) {
    pos.setX(pos.getX() + texW / 2);
    pos.setY(pos.getY() + texH / 2);
    b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
    pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Projectiles::GetPosition() {
    b2Vec2 bodyPos = pbody->body->GetTransform().p;
    Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
    return pos;
}

void Projectiles::SetDirection(bool direction) {
    isLookingLeft = direction;
}

void Projectiles::SetDirectionUp(bool direction) {
    isLookingUp = direction;
}
