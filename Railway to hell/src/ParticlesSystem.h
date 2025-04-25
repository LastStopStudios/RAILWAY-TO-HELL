#pragma once

#include <vector>
#include <SDL2/SDL.h>

// Represents a single particle
class Particle {
public:
    float x, y;         // Position
    float vx, vy;       // Velocity
    float life;         // Current life
    float maxLife;      // Maximum life
    float size;         // Size

    Particle(float x, float y, float vx, float vy, float maxLife, float size);
    void update(float deltaTime);
    float getAlpha() const;  // For fade in/out effect
};

// Particle system manager
class ParticlesSystem {
private:
    std::vector<Particle> particles;
    int maxParticles;
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;
    float speedFactor = 0.2f;

public:
    // Constructor that takes an SDL_Renderer* and optionally sets max particles
    ParticlesSystem(SDL_Renderer* renderer, int maxParticles = 200);

    // Set screen dimensions
    void setScreenDimensions(int width, int height);

    // Emit particles across the entire screen
    void emitFullScreen(int count, const SDL_Rect& camera = { 0, 0, 0, 0 });

    // Emit particles at a specific position
    void emit(float x, float y, int count, float spreadX = 30.0f, float spreadY = 30.0f);

    // Update all particles
    void update(float deltaTime);

    // Render all particles
    void render(const SDL_Rect& camera = { 0, 0, 0, 0 });

    // Clear all particles
    void clear();

    // Get number of active particles
    int getActiveCount() const;
};
