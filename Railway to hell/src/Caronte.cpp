#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Caronte.h"

Caronte::Caronte() : Entity(EntityType::CARONTE)
{
    deathTimer = 0;
    isDying = false;
}

Caronte::~Caronte() {}

bool Caronte::Awake() {
    return true;
}

bool Caronte::Start() {


    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    attack.LoadAnimations(parameters.child("animations").child("attack"));
    hurt.LoadAnimations(parameters.child("animations").child("hurt"));
    die.LoadAnimations(parameters.child("animations").child("die"));
    currentAnimation = &idle;

    // Initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), texW, texH, bodyType::KINEMATIC);
    AttackSensorArea = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)position.getY(), texW * 2, texH * 1.2, bodyType::KINEMATIC);

    pbody->listener = this;
    AttackSensorArea->listener = this;

    // Assign collider type
    pbody->ctype = ColliderType::CARONTE;
    AttackSensorArea->ctype = ColliderType::ATTACKSENSOR;

    // Fix the rotation of the body
    pbody->body->SetFixedRotation(true);
    AttackSensorArea->body->SetFixedRotation(true);

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

    return true;
}

bool Caronte::Update(float dt)
{
    // Si la entidad está marcada para eliminación, no actualizamos
    if (pendingToDelete) {
        return true;
    }

    // wait for the first input to start the game
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();
        return true;
    }

    // Manejamos la secuencia de muerte si se activó
    if (isDying) {
        currentAnimation = &die;
        deathTimer += dt;

        // Si han pasado 2 segundos desde que empezamos a morir, o la animación terminó
        if (deathTimer >= 2.0f || die.HasFinished()) {
            // Crear el item antes de marcar la entidad para eliminación
            Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
            item->SetParameters(Engine::GetInstance().scene.get()->itemConfigNode);
            Engine::GetInstance().scene.get()->itemList.push_back(item);
            item->Start();
            Vector2D pos(position.getX() + texW, position.getY());
            item->SetPosition(pos);

            // Limpiamos los cuerpos físicos antes de marcar para eliminación
            if (AttackArea != nullptr) {
                Engine::GetInstance().physics->DeletePhysBody(AttackArea);
                AttackArea = nullptr;
            }
            if (AttackSensorArea != nullptr) {
                Engine::GetInstance().physics->DeletePhysBody(AttackSensorArea);
                AttackSensorArea = nullptr;
            }
            if (pbody != nullptr) {
                Engine::GetInstance().physics->DeletePhysBody(pbody);
                pbody = nullptr;
            }

            // Marcar para eliminación
            pendingToDelete = true;
            return true;
        }
    }
    else if (hurt.HasFinished()) {
        // Activamos el modo de muerte
        isDying = true;
        deathTimer = 0.0f;
        currentAnimation = &die;
        die.Reset(); // Aseguramos que la animación empieza desde el principio
    }

    if (isattacking) {
        if (!attacked) {
            attacked = true;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation = &attack;
            currentAnimation->Reset();

            if (AttackArea == nullptr) { // create the attack area
                AttackArea = Engine::GetInstance().physics.get()->CreateRectangleSensor(
                    (int)position.getX() - 10,
                    (int)position.getY() + texH / 2,
                    70,
                    40,
                    bodyType::KINEMATIC
                );
                AttackArea->ctype = ColliderType::BOSS_ATTACK;
            }
        }
    }
    if (attack.HasFinished()) { // stop attacking
        candie = true;
        currentAnimation = &idle;
        attack.Reset();

        if (AttackArea != nullptr) { // delete the attack area
            Engine::GetInstance().physics->DeletePhysBody(AttackArea);
            AttackArea = nullptr;
        }
    }

    // update the position of caronte from the physics  
    if (pbody == nullptr) {
        LOG("Caronte pbody is null!");
        return false;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Update the attack area position of caronte
    if (AttackSensorArea != nullptr) {
        int sensorX = position.getX() + texW;
        int sensorY = position.getY() + texH * 0.4;
        AttackSensorArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(sensorX), PIXEL_TO_METERS(sensorY)), 0);
    }

    SDL_Rect frame = currentAnimation->GetCurrentFrame();

    // draw the caronte
    if (currentAnimation == &idle) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - (frame.w - texW * 1.3), (int)position.getY() + 8, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
    }
    if (currentAnimation == &attack) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW / 2, (int)position.getY() - 24, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
    }
    if (currentAnimation == &hurt) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - (frame.w - texW * 1.6), (int)position.getY() + 8, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
    }
    if (currentAnimation == &die) {
        Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW / 2, (int)position.getY() + 8, &currentAnimation->GetCurrentFrame(), 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_HORIZONTAL);
    }
    // update the animation
    currentAnimation->Update();

    return true;
}

void Caronte::OnCollision(PhysBody* physA, PhysBody* physB) {
    // Ignoramos colisiones si estamos pendientes de eliminar
    if (pendingToDelete) return;

    switch (physB->ctype) {
    case ColliderType::PLAYER:
        if (!isattacking) {
            isattacking = true;
            LOG("Attack Player!");
        }
        // Ya no detectamos la muerte aquí, sino en el Update con el timer
        break;
    case ColliderType::PLAYER_ATTACK:
        if (candie && !isDying) {
            candie = false;
            currentAnimation = &hurt;
            hurt.Reset(); // Aseguramos que la animación empieza desde el principio
        }
        break;
    case ColliderType::ITEM:
        break;
    case ColliderType::SENSOR:
        break;
    case ColliderType::ASCENSORES:
        break;
    case ColliderType::DIALOGOS:
        break;
    case ColliderType::UNKNOWN:
        break;
    }
}

void Caronte::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    // Ignoramos colisiones si estamos pendientes de eliminar
    if (pendingToDelete) return;
    // at the moment is not being used
}

bool Caronte::CleanUp()
{
    // Limpieza final de los cuerpos físicos si aún existen
    if (AttackSensorArea != nullptr) {
        Engine::GetInstance().physics->DeletePhysBody(AttackSensorArea);
        AttackSensorArea = nullptr;
    }

    if (pbody != nullptr) {
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }

    if (AttackArea != nullptr) {
        Engine::GetInstance().physics->DeletePhysBody(AttackArea);
        AttackArea = nullptr;
    }

    return true;
}