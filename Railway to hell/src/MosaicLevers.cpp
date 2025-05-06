#include "MosaicLevers.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"

MosaicLever::MosaicLever() : Entity(EntityType::LEVER), activated(false), puzzleRef(nullptr), activationCooldown(1.0f), cooldownTimer(0.0f)
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

    int firstTargetId = parameters.attribute("target_piece_id").as_int();
    if (firstTargetId > 0) {
        targetPieceIds.push_back(firstTargetId);
    }

    // Comprobar si hay piezas adicionales en el formato "target_piece_id_2", "target_piece_id_3", etc.
    for (int i = 2; i <= 5; i++) { 
        std::string attrName = "target_piece_id_" + std::to_string(i);
        pugi::xml_attribute attr = parameters.attribute(attrName.c_str());
        if (attr) {
            int additionalTargetId = attr.as_int();
            if (additionalTargetId > 0) {
                targetPieceIds.push_back(additionalTargetId);
            }
        }
        else {
            break; // Si no encontramos el atributo, salimos del bucle
        }
    }

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

    // Rotate all target pieces if puzzle reference exists
    if (puzzleRef != nullptr) {
        for (int pieceId : targetPieceIds) {
            puzzleRef->RotatePiece(pieceId);
        }
    }
    else {
        LOG("Error: MosaicLever has no puzzle reference!");
    }
}

void MosaicLever::AddTargetPiece(int pieceId)
{
    if (pieceId > 0) {
        targetPieceIds.push_back(pieceId);
    }
}

const std::vector<int>& MosaicLever::GetTargetPieceIds() const
{
    return targetPieceIds;
}

// Reemplazar el método GetTargetPieceId() (opcional, para compatibilidad)
int MosaicLever::GetTargetPieceId() const
{
    return targetPieceIds.empty() ? 0 : targetPieceIds[0];
}

void MosaicLever::SetPuzzle(MosaicPuzzle* puzzle)
{
    puzzleRef = puzzle;
}