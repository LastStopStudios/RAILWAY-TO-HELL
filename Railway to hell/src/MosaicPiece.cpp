#include "MosaicPiece.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

MosaicPiece::MosaicPiece() : Entity(EntityType::ITEM), currentRotation(0), correctRotation(0),
pieceId(0), pieceType(0), isRotating(false), rotationFrame(0)
{
}

MosaicPiece::~MosaicPiece() {}

bool MosaicPiece::Awake() {
    return true;
}

bool MosaicPiece::Start() {
    // Initialize texture
    texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
    if (texture != nullptr) {
        int w, h;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        LOG("Texture loaded successfully. Dimensions: %d x %d", w, h);
    }
    else {
        LOG("Failed to load texture!");
    }
    position.setX(parameters.attribute("x").as_int());
    position.setY(parameters.attribute("y").as_int());
    texW = parameters.attribute("w").as_int();
    texH = parameters.attribute("h").as_int();
    pieceId = parameters.attribute("piece_id").as_int();
    pieceType = parameters.attribute("piece_type").as_int(0);

    // Load the correct rotation for this piece
    correctRotation = parameters.attribute("correct_rotation").as_int();

    switch (pieceId) {
    case 1:
        currentRotation = 0; // 0 degrees
        break;
    case 2:
        currentRotation = 1; // 90 degrees
        break;
    case 3:
        currentRotation = 2; // 180 degrees
        break;
    case 4:
        currentRotation =3; // 270 degrees
        break;
    }

    // Load animations
    idle.LoadAnimations(parameters.child("animations").child("idle"));

    // Comprueba si existe la animación de rotación
    auto rotatingNode = parameters.child("animations").child("rotating");
    if (rotatingNode) {
        LOG("Loading rotation animation for piece %d", pieceId);
        rotating.LoadAnimations(rotatingNode);
    }
    else {
        LOG("Warning: No rotation animation found for piece %d", pieceId);
    }

    currentAnim = &idle;
    isRotating = false;

    // Add a physics body - initialize the physics body
    pbody = Engine::GetInstance().physics.get()->CreateRectangleSensor(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW, texH,
        bodyType::STATIC);

    pbody->listener = this;
    pbody->ctype = ColliderType::MOSAIC_PIECE;

    return true;
}

bool MosaicPiece::Update(float dt)
{
    // Actualiza la animación correspondiente
    if (isRotating) {
        rotating.Update();

        // Comprueba si la animación ha terminado
        if (rotating.HasFinished()) {
            LOG("Rotation animation finished for piece %d", pieceId);
            isRotating = false;
            currentAnim = &idle;
            int oldRotation = currentRotation;
            // Increment rotation and wrap around using modulo 4 (0, 1, 2, 3)
            currentRotation = (currentRotation + 1) % 4;
            LOG("MosaicPiece: Piece ID %d rotating from %d to %d degrees", pieceId, oldRotation * 90, currentRotation * 90);
        }
        else {
            currentAnim = &rotating;
        }
    }
    else {
        idle.Update();
        currentAnim = &idle;
    }

    // Obtén el frame actual
    SDL_Rect rect = currentAnim->GetCurrentFrame();

    // Dibuja la textura con rotación
    double angle = currentRotation * 90.0; // Convert rotation (0-3) to degrees (0, 90, 180, 270)

    // Imprime información de depuración
    if (isRotating) {
        LOG("Drawing rotation frame for piece %d: x=%d, y=%d, w=%d, h=%d",
            pieceId, rect.x, rect.y, rect.w, rect.h);
    }

    // Using correct signature: DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, 
    // float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip)
    Engine::GetInstance().render.get()->DrawTexture(
        texture,                // Texture
        (int)position.getX(),   // X position
        (int)position.getY(),   // Y position
        &rect,                  // Current animation frame
        1.0f,                   // Normal speed
        angle,                  // Rotation angle
        texW / 2,               // Pivot X (center of texture)
        texH / 2,               // Pivot Y (center of texture)
        SDL_FLIP_NONE           // No flipping
    );

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
    // Inicia la animación de rotación
    isRotating = true;
    rotating.Reset();
    currentAnim = &rotating;
    LOG("Starting rotation animation for piece %d", pieceId);

    // Play rotation sound
    Engine::GetInstance().audio.get()->PlayFx(Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/piece_rotate.wav"));
}

bool MosaicPiece::IsCorrectRotation() const
{
    return (currentRotation % 4) == (correctRotation % 4);
}

int MosaicPiece::GetRotation() const
{
    return currentRotation;
}