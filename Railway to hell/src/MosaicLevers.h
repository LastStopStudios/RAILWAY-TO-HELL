#pragma once

#include "Entity.h"
#include "Animation.h"
#include "Physics.h"
#include "MosaicPuzzle.h"

class MosaicLever : public Entity
{
public:
    MosaicLever();
    virtual ~MosaicLever();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;
    bool CleanUp() override;

    // Activate the lever to rotate a piece
    void Activate();

    // Get the target piece ID that this lever controls
    int GetTargetPieceId() const;

    // Set the puzzle this lever belongs to
    void SetPuzzle(MosaicPuzzle* puzzle);

    void AddTargetPiece(int pieceId);
    const std::vector<int>& GetTargetPieceIds() const;

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }

private:
    SDL_Texture* texture;
    Animation idle;
    Animation lever_activated;
    Animation* currentAnimation;
    PhysBody* pbody;

    bool activated;
    int texW, texH;

    std::vector<int> targetPieceIds;

    // Reference to the puzzle this lever belongs to
    MosaicPuzzle* puzzleRef;

    pugi::xml_node parameters;

    // Cooldown to prevent rapid activation
    float activationCooldown;
    float cooldownTimer;
};