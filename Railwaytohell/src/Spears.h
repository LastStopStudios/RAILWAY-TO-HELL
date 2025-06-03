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

    // Configurar direcci�n de lanzamiento
    void SetDirection(SpearDirection direction);

    // Configurar posici�n de origen personalizada
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


private:
    void CreatePhysicsBody();
    SDL_Texture* texture;
    bool isSensorBody = false;
    const char* texturePath;

    int platformCollisionCount;
    int maxPlatformCollisions;
    float lastCollisionTime;
    float collisionCooldown;
    float lifeTimer;
    float maxLifeTime;
    bool useLifeTimer;

    Animation* currentAnimation = nullptr;
    Animation falling;        // Animaci�n de 1 frame cuando est� cayendo/tir�ndose
    Animation disappear;      // Animaci�n de desaparecer

    pugi::xml_node parameters;
    PhysBody* pbody;

};