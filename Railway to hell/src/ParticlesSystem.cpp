#include "ParticlesSystem.h"
#include <algorithm>
#include <random>
#include <cmath>

// Particle constructor
Particle::Particle(float x, float y, float vx, float vy, float maxLife, float size)
    : x(x), y(y), vx(vx), vy(vy), life(maxLife), maxLife(maxLife), size(size) {
}

// Update particle
void Particle::update(float deltaTime) {
    x += vx * deltaTime;
    y += vy * deltaTime;
    life -= deltaTime;
}

float Particle::getAlpha() const {
    // Use a smoother fade in/out curve
    if (life > 0.7f * maxLife) {
        // Smooth fade-in - lasts for the first 30% of life
        float t = (maxLife - life) / (0.3f * maxLife);
        return 0.5f * (1.0f - cos(t * M_PI / 2)); // Reduced from 0.8f to 0.5f for more transparency
    }
    else if (life < 0.4f * maxLife) {
        // Smooth fade-out - lasts for the last 40% of life
        float t = life / (0.4f * maxLife);
        return 0.5f * (1.0f - cos(t * M_PI / 2)); // Reduced from 0.8f to 0.5f for more transparency
    }
    return 0.5f; // Max alpha reduced from 0.8f to 0.5f in the middle of life
}

// Particle system constructor
ParticlesSystem::ParticlesSystem(SDL_Renderer* renderer, int maxParticles)
    : renderer(renderer), maxParticles(maxParticles), screenWidth(800), screenHeight(600) {
    particles.reserve(maxParticles);
}

// Set screen dimensions
void ParticlesSystem::setScreenDimensions(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void ParticlesSystem::emitFullScreen(int count, const SDL_Rect& camera) {
    // Reduce number of particles emitted (originally was count)
    int reducedCount = count / 2;
    if (reducedCount < 1) reducedCount = 1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(camera.x, camera.x + screenWidth);
    // Particles start from above the screen
    std::uniform_real_distribution<float> disY(camera.y - screenHeight * 0.5f, camera.y);
    std::uniform_real_distribution<float> disVelX(-0.05f, 0.05f); // Small random horizontal movement
    std::uniform_real_distribution<float> disLife(1000.0f, 15000.0f); // Increased lifetime
    std::uniform_real_distribution<float> disSize(5.0f, 10.0f);

    for (int i = 0; i < reducedCount && particles.size() < maxParticles; ++i) {
        float x = disX(gen);
        float y = disY(gen);
        float vx = disVelX(gen);
        float vy = 0.1f; // Constant downward velocity (positive in SDL)
        float maxLife = disLife(gen);
        float size = disSize(gen);

        particles.emplace_back(x, y, vx, vy, maxLife, size);
    }
}

// Emit new particles at a specific position
void ParticlesSystem::emit(float x, float y, int count, float spreadX, float spreadY) {
    // Reduce the number of particles to emit
    int reducedCount = count / 2;
    if (reducedCount < 1) reducedCount = 1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(-spreadX, spreadX);
    // Adjust vertical distribution so particles start higher
    std::uniform_real_distribution<float> disY(-spreadY - 200.0f, -spreadY);
    std::uniform_real_distribution<float> disVelX(-0.05f, 0.05f);
    std::uniform_real_distribution<float> disLife(1000.0f, 15000.0f);
    std::uniform_real_distribution<float> disSize(3.0f, 10.0f);

    for (int i = 0; i < reducedCount && particles.size() < maxParticles; ++i) {
        float offsetX = disX(gen);
        float offsetY = disY(gen);
        float vx = disVelX(gen);
        float vy = 0.1f;
        float maxLife = disLife(gen);
        float size = disSize(gen);

        particles.emplace_back(x + offsetX, y + offsetY, vx, vy, maxLife, size);
    }
}

// Update all particles
void ParticlesSystem::update(float deltaTime) {
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }

    // Remove dead particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end()
    );
}

void ParticlesSystem::render(const SDL_Rect& camera) {
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

    // Use additive blending for brighter effect
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

    for (const auto& particle : particles) {
        Uint8 alpha = static_cast<Uint8>(255 * particle.getAlpha());

        // Softer and more transparent color (changed from 220,220,220 to 180,180,180)
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, alpha);

        int centerX = static_cast<int>(particle.x) - camera.x;
        int centerY = static_cast<int>(particle.y) - camera.y;
        int radius = static_cast<int>(particle.size / 2);

        // Draw a soft filled circle
        for (int w = -radius; w <= radius; w++) {
            for (int h = -radius; h <= radius; h++) {
                float distance = sqrt(w * w + h * h);
                if (distance <= radius) {
                    float edgeFade = 1.0f;
                    if (distance > radius * 0.6f) {
                        edgeFade = 1.0f - ((distance - radius * 0.6f) / (radius * 0.4f));
                    }

                    Uint8 pixelAlpha = static_cast<Uint8>(alpha * edgeFade * 0.7f);
                    SDL_SetRenderDrawColor(renderer, 180, 180, 180, pixelAlpha);
                    SDL_RenderDrawPoint(renderer, centerX + w, centerY + h);
                }
            }
        }
    }

    // Restore original blend mode
    SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}

// Clear all particles
void ParticlesSystem::clear() {
    particles.clear();
}

// Get number of active particles
int ParticlesSystem::getActiveCount() const {
    return static_cast<int>(particles.size());
}
