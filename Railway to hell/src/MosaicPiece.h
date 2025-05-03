#pragma once

#include "Entity.h"
#include "Animation.h"

class MosaicPiece : public Entity
{
public:
    MosaicPiece();
    virtual ~MosaicPiece();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;
    bool CleanUp() override;

    // Rotate the piece 90 degrees clockwise
    void Rotate();

    // Check if the piece is in the correct rotation
    bool IsCorrectRotation() const;

    // Get the current rotation (0, 1, 2, 3 representing 0°, 90°, 180°, 270°)
    int GetRotation() const;

private:
    SDL_Texture* texture;
    Animation currentAnim;
    PhysBody* pbody;

    // Animations for each rotation (0°, 90°, 180°, 270°)
    Animation rotation0;
    Animation rotation90;
    Animation rotation180;
    Animation rotation270;

    // Current rotation state (0-3)
    int currentRotation;

    // The correct rotation for this piece to solve the puzzle
    int correctRotation;

    int texW, texH;
    int pieceId;

    // Helper function to get the current animation based on rotation

    pugi::xml_node parameters;

    Animation* GetCurrentRotationAnim();
};