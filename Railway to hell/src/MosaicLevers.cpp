#include "MosaicLevers.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"

MosaicLever::MosaicLever() : Entity(EntityType::LEVER), activated(false), targetPieceId(0), puzzleRef(nullptr), activationCooldown(1.0f), cooldownTimer(0.0f)
{

}

MosaicLever::~MosaicLever() {}

bool MosaicLever::Awake() {
    return true;
}

bool MosaicLever::Start() {
    // Initialize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();

    // Get the target piece ID from parameters
    targetPieceId = parameters.attribute("target_piece_id").as_int();

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    lever_activated.LoadAnimations(parameters.child("animations").child("lever_activated"));
    currentAnimation = &idle;

    // Add physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW / 2, texH,
        bodyType::KINEMATIC);

    pbody->listener = this;
    pbody->ctype = ColliderType::LEVER;

    // Set the gravity of the body
    if (!parameters.attribute("gravity").as_bool()) {
        pbody->body->SetGravityScale(0);
    }

    return true;
}

bool MosaicLever::Update(float dt)
{
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    // Update the cooldown timer
    if (cooldownTimer > 0.0f) {
        cooldownTimer -= dt;
    }

    // Update position from physics
    if (pbody != nullptr) {
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texW / 2);
    }

    // Draw the lever
    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX(),
        (int)position.getY(),
        &currentAnimation->GetCurrentFrame()
    );

    currentAnimation->Update();

    // Reset the animation if it completed a cycle and we're showing the activation animation
    if (currentAnimation == &lever_activated && lever_activated.HasFinished()) {
        currentAnimation = &idle;
        activated = false;
    }

    return true;
}

void MosaicLever::OnCollision(PhysBody* physA, PhysBody* physB) {
    // We only care about player whip attacks or player interaction
    if (physB->ctype == ColliderType::PLAYER_WHIP_ATTACK || physB->ctype == ColliderType::PLAYER) {
        // Check if we can activate (not in cooldown)
        if (cooldownTimer <= 0.0f && !activated) {
            Activate();
        }
    }
}

void MosaicLever::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    // Nothing to do here
}

bool MosaicLever::CleanUp()
{
    if (pbody != nullptr) {
        pbody->listener = nullptr;
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }
    return true;
}

void MosaicLever::Activate()
{
    activated = true;
    currentAnimation = &lever_activated;
    cooldownTimer = activationCooldown;

    // Play activation sound
    Engine::GetInstance().audio.get()->PlayFx(Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/lever_activate.wav"));

    // Rotate the target piece if puzzle reference exists
    if (puzzleRef != nullptr) {
        puzzleRef->RotatePiece(targetPieceId);
    }
    else {
        LOG("Error: MosaicLever has no puzzle reference!");
    }
}

int MosaicLever::GetTargetPieceId() const
{
    return targetPieceId;
}

void MosaicLever::SetPuzzle(MosaicPuzzle* puzzle)
{
    puzzleRef = puzzle;
}