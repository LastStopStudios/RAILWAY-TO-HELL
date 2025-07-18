#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"

struct SDL_Texture;

enum class SpearDirection {
    HORIZONTAL_LEFT,
    HORIZONTAL_RIGHT,
    VERTICAL_DOWN,
};

class Spears : public Entity
{
public:

    Spears();
    virtual ~Spears();

    bool Awake();

    bool Start();

    bool Update(float dt);

    bool CleanUp();

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }

    void SetPosition(Vector2D pos);

    Vector2D GetPosition();

    // Set launch direction
    void SetDirection(SpearDirection direction);

    // Set custom origin position
    void SetOriginPosition(Vector2D origin);

    void OnCollision(PhysBody* physA, PhysBody* physB);

    void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

    void startDisappearAnimation();
    bool isDisappearing = false;
private:
    float moveSpeed;

    int texW, texH, texRadius;

    SpearDirection spearDirection = SpearDirection::HORIZONTAL_RIGHT;
    Vector2D originPosition;
    bool hasCustomOrigin = false;

    //Shadow cositas
    void SetupShadow();
    void RenderShadow();
    float FindGroundLevel();
    void SetHorizontalSpeed(float speed);
    void SetVerticalSpeed(float speed);
    float GetHorizontalSpeed() const;
    float GetVerticalSpeed() const;

    SDL_Texture* shadowTexture;
    bool hasShadow;
    float shadowGroundY;

private:
    void CreatePhysicsBody();
    SDL_Texture* texture;
    bool isSensorBody = false;
    const char* texturePath;

    float horizontalSpeed;  // Speed for horizontal spears
    float verticalSpeed;    // Speed for vertical spears

    int platformCollisionCount;
    int maxPlatformCollisions;
    float lastCollisionTime;
    float collisionCooldown;
    float lifeTimer;
    float maxLifeTime;
    bool useLifeTimer;

    //Destroy Spear
    int veces = 1;

    Animation* currentAnimation = nullptr;
    Animation falling;        // 1 frame animation when falling / jumping
    Animation disappear;      // Disappearing animation

    pugi::xml_node parameters;
    PhysBody* pbody;

};