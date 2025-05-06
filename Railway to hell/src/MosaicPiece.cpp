#include "MosaicPiece.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

MosaicPiece::MosaicPiece() : Entity(EntityType::ITEM), currentRotation(0), correctRotation(0), pieceId(0), pieceType(0)
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
    pieceType = parameters.attribute("piece_type").as_int(0);

    // Load the correct rotation for this piece
    correctRotation = parameters.attribute("correct_rotation").as_int();

    // Randomly set the initial rotation (0-3)
    currentRotation = rand() % 4;

    // Load single animation
    idle.LoadAnimations(parameters.child("animations").child("idle"));
    currentAnim = idle;

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

    // Get the current animation frame
    SDL_Rect frame = currentAnim.GetCurrentFrame();

    // Convert rotation index to angle in degrees
    double angle = currentRotation * 90.0;

    // Si tu motor no tiene DrawTextureEx, puedes usar esta alternativa con SDL_RenderCopyEx directamente
    SDL_Rect destRect;
    destRect.x = (int)position.getX();
    destRect.y = (int)position.getY();
    destRect.w = texW;
    destRect.h = texH;

    SDL_Point center;
    center.x = texW / 2;
    center.y = texH / 2;

    // Obtener el renderer desde el motor
    SDL_Renderer* renderer = Engine::GetInstance().render.get()->renderer;

    // Dibujar con rotación
    SDL_RenderCopyEx(
        renderer,
        texture,
        &frame,         // Source rectangle (frame de animación)
        &destRect,      // Destination rectangle
        angle,          // Angle in degrees
        &center,        // Pivot point (center of the image)
        SDL_FLIP_NONE   // No flip
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
    currentRotation = (currentRotation + 1) % 4;
   // Engine::GetInstance().audio.get()->PlayFx(Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/piece_rotate.wav"));
}

bool MosaicPiece::IsCorrectRotation() const
{
    return currentRotation == correctRotation;
}

int MosaicPiece::GetRotation() const
{
    return currentRotation;
}