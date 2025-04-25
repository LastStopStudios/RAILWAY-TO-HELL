#include "ParticlesSystem.h"
#include <algorithm>
#include <random>
#include <cmath>

// Constructor de Particle
Particle::Particle(float x, float y, float vx, float vy, float maxLife, float size)
    : x(x), y(y), vx(vx), vy(vy), life(maxLife), maxLife(maxLife), size(size) {
}

// Actualizar la part?cula
void Particle::update(float deltaTime) {
    x += vx * deltaTime;
    y += vy * deltaTime;
    life -= deltaTime;
}

float Particle::getAlpha() const {
    // Usamos una curva m?s suave para el fade in/out
    if (life > 0.7f * maxLife) {
        // Fade in con curva suavizada - dura el 30% inicial de la vida
        float t = (maxLife - life) / (0.3f * maxLife);
        return 0.8f * (1.0f - cos(t * M_PI / 2)); // Usando coseno para una transici?n m?s suave
    }
    else if (life < 0.4f * maxLife) {
        // Fade out con curva suavizada - dura el 40% final de la vida
        float t = life / (0.4f * maxLife);
        return 0.8f * (1.0f - cos(t * M_PI / 2)); // Usando coseno para una transici?n m?s suave
    }
    return 0.8f; // Alpha m?ximo reducido a 80% en la parte central de la vida
}

// Constructor del sistema de part?culas
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
    // Reducir el n?mero de part?culas a emitir (originalmente era count)
    int reducedCount = count / 2; // Reducimos a la mitad el n?mero de part?culas
    if (reducedCount < 1) reducedCount = 1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(camera.x, camera.x + screenWidth);
    // Las partículas comienzan desde más arriba (por encima de la pantalla)
    std::uniform_real_distribution<float> disY(camera.y - screenHeight * 0.5f, camera.y);
    std::uniform_real_distribution<float> disVelX(-0.05f, 0.05f); // Pequeño movimiento horizontal aleatorio
    std::uniform_real_distribution<float> disLife(1000.0f, 15000.0f); // Incrementamos la vida para que permanezcan m?s tiempo
    std::uniform_real_distribution<float> disSize(5.0f, 10.0f);

    for (int i = 0; i < reducedCount && particles.size() < maxParticles; ++i) {
        float x = disX(gen);
        float y = disY(gen);
        float vx = disVelX(gen); // Movimiento horizontal aleatorio pequeño
        float vy = 0.1f; // Velocidad constante hacia abajo (positiva en SDL)
        float maxLife = disLife(gen);
        float size = disSize(gen);

        particles.emplace_back(x, y, vx, vy, maxLife, size);
    }
}

// Emitir nuevas part?culas en posici?n espec?fica
void ParticlesSystem::emit(float x, float y, int count, float spreadX, float spreadY) {
    // Reducir el n?mero de part?culas a emitir
    int reducedCount = count / 2; // Reducimos a la mitad
    if (reducedCount < 1) reducedCount = 1;

    // Generador de n?meros aleatorios
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(-spreadX, spreadX);
    // Ajustamos la distribución vertical para que las partículas empiecen más arriba
    std::uniform_real_distribution<float> disY(-spreadY - 200.0f, -spreadY);
    std::uniform_real_distribution<float> disVelX(-0.05f, 0.05f); // Pequeño movimiento horizontal aleatorio
    std::uniform_real_distribution<float> disLife(1000.0f, 15000.0f); // Incrementamos la vida
    std::uniform_real_distribution<float> disSize(3.0f, 10.0f);

    for (int i = 0; i < reducedCount && particles.size() < maxParticles; ++i) {
        float offsetX = disX(gen);
        float offsetY = disY(gen);
        float vx = disVelX(gen); // Movimiento horizontal aleatorio pequeño
        float vy = 0.1f; // Velocidad constante hacia abajo (positiva en SDL)
        float maxLife = disLife(gen);
        float size = disSize(gen);

        particles.emplace_back(x + offsetX, y + offsetY, vx, vy, maxLife, size);
    }
}

// Actualizar todas las part?culas
void ParticlesSystem::update(float deltaTime) {
    // Actualizar part?culas existentes
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }

    // Eliminar part?culas que han terminado su vida
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end()
    );
}

void ParticlesSystem::render(const SDL_Rect& camera) {
    // Guardar el modo de mezcla actual
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

    // Configurar para blending aditivo para un efecto m?s brillante
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

    for (const auto& particle : particles) {
        // Calcular alpha basado en la vida con una transici?n m?s suave
        Uint8 alpha = static_cast<Uint8>(255 * particle.getAlpha());

        // Establecer color blanco con transparencia
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, alpha);

        // Posici?n central con offset de c?mara
        int centerX = static_cast<int>(particle.x) - camera.x;
        int centerY = static_cast<int>(particle.y) - camera.y;
        int radius = static_cast<int>(particle.size / 2);

        // Usar SDL_RenderFillCircle si est? disponible en tu versi?n de SDL
        // Si no, creamos un c?rculo suave usando un enfoque diferente

        // Dibujar un c?rculo relleno suave
        for (int w = -radius; w <= radius; w++) {
            for (int h = -radius; h <= radius; h++) {
                float distance = sqrt(w * w + h * h);
                if (distance <= radius) {
                    // A?adir un gradiente suave al borde del c?rculo
                    float edgeFade = 1.0f;
                    if (distance > radius * 0.7f) {
                        edgeFade = 1.0f - ((distance - radius * 0.7f) / (radius * 0.3f));
                    }

                    // Aplicar el gradiente al alpha
                    Uint8 pixelAlpha = static_cast<Uint8>(alpha * edgeFade);
                    SDL_SetRenderDrawColor(renderer, 220, 220, 220, pixelAlpha);
                    SDL_RenderDrawPoint(renderer, centerX + w, centerY + h);
                }
            }
        }
    }

    // Restaurar el modo de mezcla original
    SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}

// Limpiar todas las part?culas
void ParticlesSystem::clear() {
    particles.clear();
}

// Obtener n?mero de part?culas activas
int ParticlesSystem::getActiveCount() const {
    return static_cast<int>(particles.size());
}