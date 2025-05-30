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
    void CreateJumpAttack(); // New jump attack method
    void UpdateJumpAttack(float dt); // Update jump attack logic
    void UpdatePosition();
    void RenderSprite();
    void RenderShadow(); // New method to render shadow
    void ResetPath();

private:
    // Core methods for phase system
    void HandlePhase1(float distanceToPlayer, float dx, float dt);
    void HandlePhase2(float distanceToPlayer, float dx, float dt);
    void HandlePhase3(float distanceToPlayer, float dx, float dt);
    void HandleTransformation(float dt);
    void UpdatePunchAttackArea();
    void UpdateJumpAttackArea(); // New method for jump attack area
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
    Animation idle, walk, punch, defeat, transform, idle2, salto, land;

    // Physics bodies
    PhysBody* pbody;
    PhysBody* punchAttackArea;
    PhysBody* jumpAttackArea; // New attack area for jump attack

    Pathfinding* pathfinding;

    // Combat system
    int currentPhase = 1;
    bool isAttacking = false;
    bool canAttack = true;
    float attackCooldown = 3000.0f; // 3 seconds
    float currentAttackCooldown = 0.0f;

    // Jump attack system
    bool isJumping = false;
    bool isLanding = false;
    bool jumpAttackActive = false;
    float jumpStartY = 0.0f;
    float jumpHeight = 100000000000.0f; // Height of the jump in pixels
    float jumpSpeed = 30.0f; // Speed of the jump
    Vector2D shadowPosition; // Position of the shadow on ground
    Vector2D targetLandingPos; // Where the devil will land
    bool shadowVisible = false;
    bool jumpStarted = false;        // Track if actual jump physics have started
    bool isOnGround = false;         // Track ground contact
    bool landingFrameHeld = false;   // Track if we're holding the first landing frame

    // State management
    bool isDying = false;
    bool Hiteado = false; // Collision flag
    bool isTransforming = false;

    // Lives system
    int lives = 3;
    int live1 = 1, live2, live3; // Phase 1: 1 hit to transform, Phase 2: 1 hit to die

};