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
    //texture = Engine::GetInstance().textures.get()->Load("Assets/Sprites/Heroes/Wizzard/BolaDeFuego.png");
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    texRadius = parameters.attribute("radius").as_int();

    //Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
	speedingUpp.LoadAnimations(parameters.child("animations").child("speedingUp"));
    moving.LoadAnimations(parameters.child("animations").child("moving"));
	impact.LoadAnimations(parameters.child("animations").child("impact"));
    currentAnimation = &idle;

    // L08 TODO 4: Add a physics to an item - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

    // L08 TODO 7: Assign collider type
    pbody->ctype = ColliderType::PROJECTILE;

    pbody->listener = this;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

    moveSpeed = 400.5f;

    return true;
}

bool Projectiles::Update(float dt)
{

    //if (isImpacting) {
    //    if (currentAnimation->HasFinished()) {
    //        Engine::GetInstance().entityManager.get()->DestroyEntity(this);
    //        return false;
    //    }
    //}
    if (!isImpacting) {
        // Actualizar la velocidad del cuerpo físico en Box2D
        if (!isLookingLeft) pbody->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(-moveSpeed), pbody->body->GetLinearVelocity().y));
        else pbody->body->SetLinearVelocity(b2Vec2(PIXEL_TO_METERS(moveSpeed), pbody->body->GetLinearVelocity().y));
    }
    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (isLookingLeft) {
        flip = SDL_FLIP_HORIZONTAL;
    }


    //Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);
    Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, flip);
    currentAnimation->Update();

    return true;
}

void Projectiles::startImpactAnimation() {
    pbody->body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    //currentAnimation = &impact;
    //currentAnimation->Reset();
    //isImpacting = true;

    Engine::GetInstance().entityManager.get()->DestroyEntity(this);

}

bool Projectiles::CleanUp()
{
    Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
    return true;
}


void Projectiles::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Collision PLATFORM");
        startImpactAnimation();
        //Engine::GetInstance().entityMa nager.get()->DestroyEntity(this);
        break;
    case ColliderType::ITEM:
        LOG("Collision ITEM");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::TERRESTRE:
        LOG("Collision TERRESTRE");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::VOLADOR:
        LOG("Collision VOLADOR");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::CARONTE:
        LOG("Collision CARONTE");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::BOSS:
        LOG("Collision BOSS");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::PLAYER:
        LOG("Collision PLAYER");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
        break;
    case ColliderType::UNKNOWN:
        LOG("Collision UNKNOWN");
        startImpactAnimation();
        //Engine::GetInstance().entityManager.get()->DestroyEntity(this);
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
    case ColliderType::ENEMY:
        LOG("Collision ENEMY");
        break;
    case ColliderType::VOLADOR:
        LOG("Collision VOLADOR");
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
