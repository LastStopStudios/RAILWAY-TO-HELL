#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "DialogoM.h"
#include "UI.h"

struct SDL_Texture;

class Devil : public Entity
{
public:
    Devil();
    virtual ~Devil();
    bool Awake();
    bool Start();
    bool Update(float dt);
    bool CleanUp();

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }

    void SetPosition(Vector2D pos);
    Vector2D GetPosition();
    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    // Getters
    bool IsEnabled() const { return isEnabled; }
    std::string GetRef() { return ref; }
    int GetCurrentLives() { return lives; }
    int GetCurrentPhase() { return currentPhase; }

    // Utility methods
    void ResetLives();

    // Combat methods
    void CreatePunchAttack();
    void UpdatePosition();
    void RenderSprite();

private:
    // Core methods for phase system
    void HandlePhase1(float distanceToPlayer, float dx, float dt);
    void HandlePhase2(float distanceToPlayer, float dx, float dt);
    void HandleTransformation(float dt);
    void UpdatePunchAttackArea();
    void ResizeCollisionForPhase2();

    // Entity identification
    std::string enemyID;
    std::string ref;
    bool isEnabled = true;

    // Position and movement
    Vector2D enemyPos;
    float moveSpeed = 2.0f;
    float patrolSpeed = 3.0f;
    bool isLookingLeft = false;
    int initX, initY;

    // Rendering
    SDL_Texture* texture;
    int texW, texH;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    // Configuration
    pugi::xml_node parameters;

    // Animation system
    Animation* currentAnimation = nullptr;
    Animation idle, walk, punch, defeat, transform, idle2;

    // Physics bodies
    PhysBody* pbody;
    PhysBody* punchAttackArea;

    // Combat system
    int currentPhase = 1;
    bool isAttacking = false;
    bool canAttack = true;
    float attackCooldown = 3000.0f; // 3 seconds
    float currentAttackCooldown = 0.0f;

    // State management
    bool isDying = false;
    bool Hiteado = false; // Collision flag
    bool isTransforming = false;

    // Lives system
    int lives = 2; // Phase 1: 1 hit to transform, Phase 2: 1 hit to die
};