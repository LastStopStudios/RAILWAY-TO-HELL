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
    void ResetPath();
    void OnCollision(PhysBody* physA, PhysBody* physB);
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    // Load & save methods
    void SetDeathInXML();
    void SetAliveInXML();
    void SetSavedDeathToDeathInXML();
    void SetSavedDeathToAliveInXML();
    void SetEnabled(bool active);

    // Getters
    bool IsEnabled() const { return isEnabled; }
    std::string GetRef() { return ref; }
    int GetCurrentLives() { return lives; }
    int GetInitX() { return initX; }
    int GetInitY() { return initY; }

    // Utility methods
    void SavePosition(std::string name);
    void ResetLives();
    void ResetPosition();

    // Combat methods
    void CreatePunchAttack();
    void HandleDefeatState();
    void UpdatePosition();
    void RenderSprite();

public:
    // Load & save variables
    bool pendingDisable = false;
    int DeathValue = 0;
    int SavedDeathValue = 0;
    bool itemCreated = false;

    // Death variables
    int a = 0;
    int kill = 1;

private:
    // Entity identification
    std::string enemyID;
    std::string ref;
    bool isEnabled = true;

    // Position and movement
    Vector2D enemyPos;
    float moveSpeed;
    float patrolSpeed;
    float savedPosX;
    bool patroling = false;
    bool moving = false;
    bool isLookingLeft = false;

    // Rendering
    SDL_Texture* texture;
    const char* texturePath;
    int texW, texH;
    int initX, initY;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    // Configuration
    pugi::xml_node parameters;

    // Animation system
    Animation* currentAnimation = nullptr;
    Animation idle, walk, punch, defeat;

    // Physics bodies
    PhysBody* pbody;
    PhysBody* jumpAttackArea;
    PhysBody* punchAttackArea; // New punch attack area

    // AI and pathfinding
    Pathfinding* pathfinding;
    float pathfindingTimer = 0.0f;
    float maxPathfindingTime = 700.0f;

    // Combat system
    int currentPhase;
    bool isAttacking = false;
    bool isJumpAttacking = false;
    bool canAttack = true;
    float attackCooldown = 3000.0f; // 3 seconds
    float currentAttackCooldown = 0.0f;
    float attackDistance = 14.0f;
    int attackCounter = 0;

    // State management
    bool resting = false;
    bool escaping = false;
    bool isRunning = false;
    bool ishurt = false;
    bool isDying = false;
    bool isDead = false;
    bool Hiteado = false; // Collision flag

    // Boss specific
    int lives = 10;
    bool changeMusicBoss = false;
    float intitalPosX = 0;

    // Phase control (for future expansion)
    bool phase_One = false;
    bool phase_Two = false;
    bool phase_Three = false;

    // Jump system (for future use)
    bool isJumping = false;
    float jumpTimer = 0.0f;
    Vector2D jumpStartPos;
    Vector2D jumpTargetPos;
    float jumpDuration = 0.0f;

    // Death timing
    float deathTimer = 0.0f;
    const float deathDelay = 1.0f;
    bool dialogTriggered = false;
    bool battleStarted = false;

    // Helper methods for animation
    int GetTotalFrames() const;
    int GetCurrentFrameId() const;
};