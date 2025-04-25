#include "ParticlesSystem.h"
#include <algorithm>
#include <random>
#include <cmath>

// Constructor de Particle
Particle::Particle(float x, float y, float vx, float vy, float maxLife, float size)
    : x(x), y(y), vx(vx), vy(vy), life(maxLife), maxLife(maxLife), size(size) {
}

// Actualizar la partícula
void Particle::update(float deltaTime) {
    x += vx * deltaTime;
    y += vy * deltaTime;
    life -= deltaTime;
}

// Calcular alpha para el fade in/out
float Particle::getAlpha() const {
    // Fade in durante el primer 20% de vida
    // Fade out durante el último 30% de vida
    if (life > 0.8f * maxLife) {
        return 1.0f - (maxLife - life) / (0.2f * maxLife); // Fade in
    }
    else if (life < 0.3f * maxLife) {
        return life / (0.3f * maxLife); // Fade out
    }
    return 1.0f; // Alpha completo en medio de la vida
}

// Constructor del sistema de partículas
ParticlesSystem::ParticlesSystem(SDL_Renderer* renderer, int maxParticles)
    : renderer(renderer), maxParticles(maxParticles), screenWidth(800), screenHeight(600) {
    particles.reserve(maxParticles);
}

// Establecer dimensiones de la pantalla
void ParticlesSystem::setScreenDimensions(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void ParticlesSystem::emitFullScreen(int count, const SDL_Rect& camera) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(camera.x, camera.x + screenWidth);
    std::uniform_real_distribution<float> disY(camera.y, camera.y + screenHeight);
    std::uniform_real_distribution<float> disVel(-0.1f, 0.1f);
    std::uniform_real_distribution<float> disLife(500.0f, 10000.0f);
    std::uniform_real_distribution<float> disSize(5.0f, 10.0f); // Partículas más grandes

    for (int i = 0; i < count && particles.size() < maxParticles; ++i) {
        float x = disX(gen);
        float y = disY(gen);
        float vx = disVel(gen);
        float vy = disVel(gen);
        float maxLife = disLife(gen);
        float size = disSize(gen);

        particles.emplace_back(x, y, vx, vy, maxLife, size);
    }
}

// Emitir nuevas partículas en posición específica
void ParticlesSystem::emit(float x, float y, int count, float spreadX, float spreadY) {
    // Generador de números aleatorios
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(-spreadX, spreadX);
    std::uniform_real_distribution<float> disY(-spreadY, spreadY);
    std::uniform_real_distribution<float> disLife(500.0f, 10000.0f);
    std::uniform_real_distribution<float> disSize(3.0f, 10.0f);

    for (int i = 0; i < count && particles.size() < maxParticles; ++i) {
        float vx = disX(gen) * 0.5f;
        float vy = disY(gen) * 0.5f;
        float maxLife = disLife(gen);
        float size = disSize(gen);

        particles.emplace_back(x, y, vx, vy, maxLife, size);
    }
}

// Actualizar todas las partículas
void ParticlesSystem::update(float deltaTime) {
    // Actualizar partículas existentes
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }

    // Eliminar partículas que han terminado su vida
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end()
    );
}

// Renderizar todas las partículas con offset de cámara
void ParticlesSystem::render(const SDL_Rect& camera) {
    // Guardar el modo de mezcla actual
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

    // Configurar para blending aditivo para un efecto más brillante
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

    for (const auto& particle : particles) {
        // Calcular alpha basado en la vida
        Uint8 alpha = static_cast<Uint8>(255 * particle.getAlpha());

        // Dibujar partícula como un punto blanco con transparencia
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);

        // Dibujar la partícula como un cuadrado pequeño, aplicando offset de cámara
        SDL_Rect rect = {
            static_cast<int>(particle.x - particle.size / 2) - camera.x,
            static_cast<int>(particle.y - particle.size / 2) - camera.y,
            static_cast<int>(particle.size),
            static_cast<int>(particle.size)
        };
        SDL_RenderFillRect(renderer, &rect);
    }

    // Restaurar el modo de mezcla original
    SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}

// Limpiar todas las partículas
void ParticlesSystem::clear() {
    particles.clear();
}

// Obtener número de partículas activas
int ParticlesSystem::getActiveCount() const {
    return static_cast<int>(particles.size());
}