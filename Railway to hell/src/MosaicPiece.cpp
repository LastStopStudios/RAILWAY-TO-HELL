#include "MosaicPiece.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

MosaicPiece::MosaicPiece() : Entity(EntityType::ITEM), currentRotation(0), correctRotation(0), pieceId(0)
{
}

MosaicPiece::~MosaicPiece() {}

bool MosaicPiece::Awake() {
    return true;
}

bool MosaicPiece::Start() {
    // Initialize textures
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    pieceId = parameters.attribute("piece_id").as_int();

    // Load the correct rotation for this piece
    correctRotation = parameters.attribute("correct_rotation").as_int();

    // Randomly set the initial rotation (0-3)
    currentRotation = rand() % 4;

    // Load animations
    rotation0.LoadAnimations(parameters.child("animations").child("rotation0"));
    rotation90.LoadAnimations(parameters.child("animations").child("rotation90"));
    rotation180.LoadAnimations(parameters.child("animations").child("rotation180"));
    rotation270.LoadAnimations(parameters.child("animations").child("rotation270"));

    // Set initial animation
    currentAnim = *GetCurrentRotationAnim();

    // Add a physics body - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW, texH,
        bodyType::STATIC);

    pbody->listener = this;
    pbody->ctype = ColliderType::ITEM;

    return true;
}

bool MosaicPiece::Update(float dt)
{
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    // Update position from physics
    if (pbody != nullptr) {
        b2Transform pbodyPos = pbody->body->GetTransform();
        position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
        position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
    }

    // Draw the mosaic piece with the current rotation
    Engine::GetInstance().render.get()->DrawTexture(
        texture,
        (int)position.getX(),
        (int)position.getY(),
        &GetCurrentRotationAnim()->GetCurrentFrame()
    );

    currentAnim.Update();
    return true;
}

void MosaicPiece::OnCollision(PhysBody* physA, PhysBody* physB) {
    // Mosaic pieces don't react to collisions directly
    // They are controlled by levers
}

void MosaicPiece::OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
    // Nothing to do here
}

bool MosaicPiece::CleanUp()
{
    if (pbody != nullptr) {
        pbody->listener = nullptr;
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }
    return true;
}

void MosaicPiece::Rotate()
{
    // Rotate 90 degrees clockwise
    currentRotation = (currentRotation + 1) % 4;

    // Update the current animation based on new rotation
    currentAnim = *GetCurrentRotationAnim();

    // Play rotation sound if needed
    Engine::GetInstance().audio.get()->PlayFx(Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/piece_rotate.wav"));
}

bool MosaicPiece::IsCorrectRotation() const
{
    return currentRotation == correctRotation;
}

int MosaicPiece::GetRotation() const
{
    return currentRotation;
}

Animation* MosaicPiece::GetCurrentRotationAnim()
{
    switch (currentRotation) {
    case 0: return &rotation0;
    case 1: return &rotation90;
    case 2: return &rotation180;
    case 3: return &rotation270;
    default: return &rotation0;
    }
}