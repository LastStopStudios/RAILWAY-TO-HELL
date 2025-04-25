#pragma once

#include <vector>
#include <SDL2/SDL.h>

class Particle {
public:
    float x, y;         // Posición
    float vx, vy;       // Velocidad
    float life;         // Vida actual
    float maxLife;      // Vida máxima
    float size;         // Tamaño

    Particle(float x, float y, float vx, float vy, float maxLife, float size);
    void update(float deltaTime);
    float getAlpha() const;  // Para el fade in/out
};

class ParticlesSystem {
private:
    std::vector<Particle> particles;
    int maxParticles;
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;

public:
    // Constructor que toma un SDL_Renderer* y dimensiones de pantalla
    ParticlesSystem(SDL_Renderer* renderer, int maxParticles = 200);

    // Establece dimensiones de la pantalla
    void setScreenDimensions(int width, int height);

    // Emitir partículas en toda la pantalla
    void emitFullScreen(int count, const SDL_Rect& camera = { 0, 0, 0, 0 });

    // Método principal para emitir partículas en una posición específica
    void emit(float x, float y, int count, float spreadX = 30.0f, float spreadY = 30.0f);

    // Actualiza todas las partículas
    void update(float deltaTime);

    // Dibuja todas las partículas
    void render(const SDL_Rect& camera = { 0, 0, 0, 0 });

    // Elimina todas las partículas
    void clear();

    // Devuelve número de partículas activas
    int getActiveCount() const;
};