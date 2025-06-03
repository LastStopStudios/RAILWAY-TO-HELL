#pragma once
#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "DialogoM.h"
#include "UI.h"
#include "Spears.h"
#include "Engine.h"

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
    void CreateJumpAttack();
    void UpdateJumpAttack(float dt);
    void UpdatePosition();
    void RenderSprite();
    void RenderShadow();
    void ResetPath();

private:
    // Phase-specific behavior handlers
    void HandlePhase1(float distanceToPlayer, float dx, float dt);
    void HandlePhase2(float distanceToPlayer, float dx, float dt);
    void HandlePhase3(float distanceToPlayer, float dx, float dt);
    void HandleTransformation(float dt);
    void UpdatePunchAttackArea();
    void UpdateJumpAttackArea();
    void ResizeCollisionForPhase2();
    void ResizeCollisionForPhase3(); 

    // Entity identification
    std::string enemyID;
    std::string ref;
    bool isEnabled = true;

    // Position and movement
    Vector2D enemyPos;
    float moveSpeed = 2.0f;
    float patrolSpeed = 3.0f;
    bool isLookingLeft = false;
    bool isChasing = false;
    int initX, initY;

    // Rendering
    SDL_Texture* texture;
    SDL_Texture* shadowTexture;
    int texW, texH;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    pugi::xml_node parameters;

    int transformStep = 0;
    float transformTimer = 0.0f;
    bool transformationStarted = false;
    bool zooming = true;
    float zoom;

    // Animation system
    Animation* currentAnimation = nullptr;
    Animation idle, walk, punch, defeat, transform, idle2, salto, land, colatazo, transform2, idle3, attackH, attackV ;

    // Physics bodies
    PhysBody* pbody;
    PhysBody* punchAttackArea;
    PhysBody* jumpAttackArea;

    Pathfinding* pathfinding;
    Spears* spears;

    // Combat system
    int currentPhase = 1;
    bool isAttacking = false;
    bool canAttack = true;
    float attackCooldown = 100.0f;
    float currentAttackCooldown = 0.0f;

    bool isSpearAttacking = false;
    bool isVerticalSpearAttack = false;
    bool isHorizontalSpearAttack = false;
    float currentSpearCooldown = 0.0f;
    float spearAttackCooldown = 3.0f; // 3 seconds between attacks
    std::vector<Spears*> activeSpears;

    // Method declarations
    void CreateVerticalSpearAttack();
    void CreateHorizontalSpearAttack();
    void UpdateSpearAttacks(float dt);
    void CleanupSpears();
    pugi::xml_node spearTemplateNode;
    pugi::xml_document spearConfigDoc;

    // Jump attack system
    bool isJumping = false;
    bool isLanding = false;
    bool jumpAttackActive = false;
    Vector2D jumpStartPos;
    Vector2D targetLandingPos;
    Vector2D shadowPosition;
    bool shadowVisible = false;
    bool hasReachedPeak = false;
    bool startFalling = false;
    float targetPlayerX = 0.0f;
    bool jumpPreparation = false;
    bool landingComplete = false;
    bool fallAnimationLocked = false;
    bool jumpAnimationLocked = false;
    bool horizontalMovementStarted = false;
    float horizontalDistance = 0.0f;
    float maxJumpHeight = 1000.0f;
    float currentJumpHeight = 0.0f;
    bool hasReachedMaxHeight = false;

    // Tail attack system
    void CreateTailAttack();
    void UpdateTailAttackArea();
    PhysBody* tailAttackArea;
    bool isTailAttacking = false;

    // State management
    bool isDying = false;
    bool Hiteado = false;
    bool isTransforming = false;
    bool pendingTransformation = false;  

    // Lives system
    int lives = 3;
    int live1 = 1, live2 = 6, live3 = 7;
};