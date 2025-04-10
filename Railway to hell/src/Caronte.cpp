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

    // Initilize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

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
    // If entity is marked for deletion, skip update
    if (pendingToDelete) {
        return true;
    }

    // Wait until the game has started (first input)
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY) {
        return true;
    }

    if (Engine::GetInstance().scene->IsSkippingFirstInput()) {
        Engine::GetInstance().scene->ResetSkipInput();
        return true;
    }

    // If the hurt animation has finished, switch to death animation
    if (currentAnimation == &hurt && hurt.HasFinished()) {
        LOG("Hurt animation finished, starting death animation");
        currentAnimation = &die;
        die.Reset();
        deathAnimationPlaying = true;
    }

    // If death animation has finished, spawn the key and mark for deletion
    if (deathAnimationPlaying && die.HasFinished()) {
        LOG("Death animation finished, dropping key");

        // Create the key item before deleting the entity
        Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
        item->SetParameters(Engine::GetInstance().scene.get()->itemConfigNode);
        Engine::GetInstance().scene.get()->itemList.push_back(item);
        item->Start();
        Vector2D pos(position.getX() + texW, position.getY());
        item->SetPosition(pos);

        // Clean up all physics bodies
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

        // Mark the entity for deletion
        pendingToDelete = true;
        return true;
    }

    // Attack logic
    if (isattacking) {
        if (!attacked) {
            attacked = true;
            canAttack = false;
            currentAttackCooldown = attackCooldown;
            currentAnimation = &attack;
            currentAnimation->Reset();

            // Create attack area only once per attack
            if (AttackArea == nullptr) {
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

    // If the attack animation finished, reset and return to idle
    if (attack.HasFinished() && currentAnimation == &attack) {
        candie = true;
        currentAnimation = &idle;
        attack.Reset();
        isattacking = false;
        attacked = false;

        // Remove attack area
        if (AttackArea != nullptr) {
            Engine::GetInstance().physics->DeletePhysBody(AttackArea);
            AttackArea = nullptr;
        }
    }

    // Update position based on physics body
    if (pbody == nullptr) {
        LOG("Caronte pbody is null!");
        return false;
    }

    b2Transform pbodyPos = pbody->body->GetTransform();
    position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
    position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

    // Update the attack sensor position (e.g., for detecting player proximity)
    if (AttackSensorArea != nullptr) {
        int sensorX = position.getX() + texW;
        int sensorY = position.getY() + texH * 0.4;
        AttackSensorArea->body->SetTransform(b2Vec2(PIXEL_TO_METERS(sensorX), PIXEL_TO_METERS(sensorY)), 0);
    }

    SDL_Rect frame = currentAnimation->GetCurrentFrame();

    // Render Caronte depending on the current animation
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

    // Update the animation frame
    currentAnimation->Update();

    return true;
}

void Caronte::OnCollision(PhysBody* physA, PhysBody* physB) {
    // Ignore collisions if pending deletion or playing death animation
    if (pendingToDelete || deathAnimationPlaying) return;

    switch (physB->ctype) {
    case ColliderType::PLAYER:
        if (!isattacking) {
            isattacking = true;
            LOG("Attack Player!");
        }
        break;
    case ColliderType::PLAYER_ATTACK:
        if (candie && currentAnimation != &hurt) {
            LOG("Caronte hit by player attack!");
            candie = false;
            currentAnimation = &hurt;
            hurt.Reset(); // Ensure the animation starts from the beginning
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
    if (pendingToDelete) return;
    // at the moment is not being used
}
bool Caronte::CleanUp()
{
    // Final cleanup of physics bodies if they still exist
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
