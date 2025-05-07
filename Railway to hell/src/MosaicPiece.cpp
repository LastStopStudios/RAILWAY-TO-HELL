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
    pbody->ctype = ColliderType::MOSAIC_PIECE;

    return true;
}

bool MosaicPiece::Update(float dt)
{
    // Dibujar la textura con rotación
    double angle = currentRotation * 90.0; // Convertir la rotación (0-3) a ángulos (0, 90, 180, 270)

    // Usando la firma correcta: DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, 
    // float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip)
    Engine::GetInstance().render.get()->DrawTexture(
        texture,                // Textura
        (int)position.getX(),   // Posición X
        (int)position.getY(),   // Posición Y
        nullptr,                // Sin sección específica (textura completa)
        1.0f,                   // Velocidad normal
        angle,                  // Ángulo de rotación
        texW / 2,               // Punto de pivote X (centro de la textura)
        texH / 2,               // Punto de pivote Y (centro de la textura)
        SDL_FLIP_NONE           // Sin voltear la imagen
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
    int oldRotation = currentRotation;
    currentRotation = (currentRotation + 1) % 4;
    LOG("MosaicPiece: Pieza ID %d rotando de %d a %d grados", pieceId, oldRotation * 90, currentRotation * 90);

    // Reproducir sonido de rotación
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