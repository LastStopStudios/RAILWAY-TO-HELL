#pragma once

#include "Entity.h"
#include "Animation.h"
#include "Physics.h"

class MosaicPiece : public Entity
{
public:
    MosaicPiece();
    virtual ~MosaicPiece();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

    // Rotate the piece 90 degrees clockwise
    void Rotate();

    // Check if the piece is in the correct rotation
    bool IsCorrectRotation() const;

    // Get the current rotation (0-3) 0=0, 1=90, 2=180, 3=270
    int GetRotation() const;

    // Get piece ID
    int GetPieceId() const { return pieceId; }

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }

private:
    SDL_Texture* texture;
    PhysBody* pbody;

    Animation* currentAnimation = nullptr;

    // Dimensions
    int texW, texH;

    // Only one animation is needed for the rotation
    Animation idle;
    Animation currentAnim;

    pugi::xml_node parameters;

    // Rotation states
    int currentRotation;  // 0=0, 1=90, 2=180, 3=270
    int correctRotation;  // The rotation needed to solve the puzzle

    // Identifier for this piece
    int pieceId;

    // Type of piece (determines the sprite frame)
    int pieceType;
};