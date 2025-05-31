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
    void CreateJumpAttack();
    void UpdateJumpAttack(float dt);
    void UpdatePosition();
    void RenderSprite();
    void RenderShadow();
    void ResetPath();

private:
    // Core methods for phase system
    void HandlePhase1(float distanceToPlayer, float dx, float dt);
    void HandlePhase2(float distanceToPlayer, float dx, float dt);
    void HandlePhase3(float distanceToPlayer, float dx, float dt);
    void HandleTransformation(float dt);
    void UpdatePunchAttackArea();
    void UpdateJumpAttackArea();
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
    bool isChasing = false;
    int initX, initY;

    // Rendering
    SDL_Texture* texture;
    SDL_Texture* shadowTexture; // New shadow texture
    int texW, texH;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    // Configuration
    pugi::xml_node parameters;

    // Animation system
    Animation* currentAnimation = nullptr;
    Animation idle, walk, punch, defeat, transform, idle2, salto, land, colatazo, transform2, idle3, attack;

    // Physics bodies
    PhysBody* pbody;
    PhysBody* punchAttackArea;
    PhysBody* jumpAttackArea;

    Pathfinding* pathfinding;

    // Combat system
    int currentPhase = 1;
    bool isAttacking = false;
    bool canAttack = true;
    float attackCooldown = 1000.0f; // 3 seconds
    float currentAttackCooldown = 0.0f;

    // Jump attack variables
    bool isJumping = false;
    bool isLanding = false;
    bool jumpAttackActive = false;
    Vector2D jumpStartPos;        // Posición inicial del salto
    Vector2D targetLandingPos;    // Posición objetivo (jugador)
    Vector2D shadowPosition;      // Posición de la sombra
    bool shadowVisible = false;
    bool hasReachedPeak = false;        // Si ya llegó al punto más alto del salto
    bool startFalling = false;         // Si debe empezar a caer en picado
    float targetPlayerX = 0.0f;        // Posición X objetivo del jugador

    // Nuevas variables para el movimiento horizontal
    bool horizontalMovementStarted = false;
    float horizontalDistance = 0.0f;

    // Cosas para el ataque de cola de la F2
    void CreateTailAttack();
    void UpdateTailAttackArea();
    PhysBody* tailAttackArea;
    bool isTailAttacking = false;

    // State management
    bool isDying = false;
    bool Hiteado = false; // Collision flag
    bool isTransforming = false;

    // Lives system
    int lives = 3;
    int live1 = 1, live2 = 6, live3 = 7; // Phase 1: 1 hit to transform, Phase 2: 2 hit to phase 3, phase 3 3 hits to die

};