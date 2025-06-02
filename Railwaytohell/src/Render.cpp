#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include "GlobalSettings.h"

#define VSYNC true

Render::Render() : Module()
{
    name = "render";
    background.r = 0;
    background.g = 0;
    background.b = 0;
    background.a = 0;
}

Render::~Render() {}

bool Render::Awake()
{
    LOG("Create SDL rendering context");
    bool ret = true;

    Uint32 flags = SDL_RENDERER_ACCELERATED;

    // Enable VSync if specified in config
    if (configParameters.child("vsync").attribute("value").as_bool() == true) {
        flags |= SDL_RENDERER_PRESENTVSYNC;
        LOG("Using vsync");
    }

    int scale = Engine::GetInstance().window.get()->GetScale();
    SDL_Window* window = Engine::GetInstance().window.get()->window;

    renderer = SDL_CreateRenderer(window, -1, flags);

    if (renderer == NULL)
    {
        LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
        ret = false;
    }
    else
    {
        camera.w = Engine::GetInstance().window.get()->width * scale;
        camera.h = Engine::GetInstance().window.get()->height * scale;
        camera.x = 0;
        camera.y = 0;
    }

    // Initialize font library
    TTF_Init();
    font = TTF_OpenFont("Assets/Fonts/PanasChill.ttf", 13);

    return ret;
}

bool Render::Start()
{
    LOG("Render start");
    SDL_RenderGetViewport(renderer, &viewport);
    return true;
}

bool Render::PreUpdate()
{
    SDL_RenderClear(renderer);
    return true;
}

bool Render::Update(float dt)
{
    return true;
}

bool Render::PostUpdate()
{
    SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
    SDL_RenderPresent(renderer);
    return true;
}

bool Render::CleanUp()
{
    LOG("Destroying SDL render");
    SDL_DestroyRenderer(renderer);
    return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
    background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
    SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
    SDL_RenderSetViewport(renderer, &viewport);
}

bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip) const
{
	bool ret = true;

	SDL_Rect rect;

	// Apply scaling to both position and texture size
	// Scale the x and y coordinates by TEXTURE_SIZE_MULTIPLIER
    float textureMultiplier = GlobalSettings::GetInstance().GetTextureMultiplier();

	rect.x = (int)(camera.x * speed) + (int)(x * textureMultiplier);
	rect.y = (int)(camera.y * speed) + (int)(y * textureMultiplier);

	if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	// Scale the texture size
	rect.w = (int)(rect.w * textureMultiplier);
	rect.h = (int)(rect.h * textureMultiplier);

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		// Scale the pivot point too
		pivot.x = (int)(pivotX * textureMultiplier);
		pivot.y = (int)(pivotY * textureMultiplier);
		p = &pivot;
	}

	if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawTextureForButtons(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip) const
{
    bool ret = true;

    SDL_Rect rect;

    rect.x = (int)(camera.x * speed) + (int)(x - 50);
    rect.y = (int)(camera.y * speed) + (int)(y - 25);

    if (section != NULL)
    {
        rect.w = section->w;
        rect.h = section->h;
    }
    else
    {
        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    }

    rect.w = (int)(rect.w );
    rect.h = (int)(rect.h );

    SDL_Point* p = NULL;
    SDL_Point pivot;

    if (pivotX != INT_MAX && pivotY != INT_MAX)
    {
        pivot.x = (int)(pivotX);
        pivot.y = (int)(pivotY);
        p = &pivot;
    }

    if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
    {
        LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawTextureForCheckBox(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip) const
{
    bool ret = true;

    SDL_Rect rect;

    rect.x = (int)(camera.x * speed) + (int)(x - 2);
    rect.y = (int)(camera.y * speed) + (int)(y );

    if (section != NULL)
    {
        rect.w = section->w;
        rect.h = section->h;
    }
    else
    {
        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    }

    rect.w = (int)(rect.w);
    rect.h = (int)(rect.h);

    SDL_Point* p = NULL;
    SDL_Point pivot;

    if (pivotX != INT_MAX && pivotY != INT_MAX)
    {
        pivot.x = (int)(pivotX);
        pivot.y = (int)(pivotY);
        p = &pivot;
    }

    if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
    {
        LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawTextureWithFlip(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip) const
{
    return DrawTexture(texture, x, y, section, speed, angle, pivotX, pivotY, flip);
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
    bool ret = true;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    float textureMultiplier = GlobalSettings::GetInstance().GetTextureMultiplier();

    SDL_Rect rec;
    if (use_camera)
    {
        rec.x = (int)(camera.x + rect.x * textureMultiplier);
        rec.y = (int)(camera.y + rect.y * textureMultiplier);
    }
    else
    {
        rec.x = (int)(rect.x * textureMultiplier);
        rec.y = (int)(rect.y * textureMultiplier);
    }
    rec.w = (int)(rect.w * textureMultiplier);
    rec.h = (int)(rect.h * textureMultiplier);

    int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

    if (result != 0)
    {
        LOG("Cannot draw rectangle. SDL_RenderFillRect error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
    bool ret = true;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    float textureMultiplier = GlobalSettings::GetInstance().GetTextureMultiplier();

    int result = -1;

    if (use_camera)
    {
        result = SDL_RenderDrawLine(renderer,
            (int)(camera.x + x1 * textureMultiplier),
            (int)(camera.y + y1 * textureMultiplier),
            (int)(camera.x + x2 * textureMultiplier),
            (int)(camera.y + y2 * textureMultiplier));
    }
    else
    {
        result = SDL_RenderDrawLine(renderer,
            (int)(x1 * textureMultiplier),
            (int)(y1 * textureMultiplier),
            (int)(x2 * textureMultiplier),
            (int)(y2 * textureMultiplier));
    }

    if (result != 0)
    {
        LOG("Cannot draw line. SDL_RenderDrawLine error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
    bool ret = true;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    float textureMultiplier = GlobalSettings::GetInstance().GetTextureMultiplier();

    SDL_Point points[360];
    float factor = (float)M_PI / 180.0f;

    int scaled_x = (int)(x * textureMultiplier);
    int scaled_y = (int)(y * textureMultiplier);
    int scaled_radius = (int)(radius * textureMultiplier);

    for (int i = 0; i < 360; ++i)
    {
        points[i].x = (int)(scaled_x + (use_camera ? camera.x : 0)) + (int)(scaled_radius * cos(i * factor));
        points[i].y = (int)(scaled_y + (use_camera ? camera.y : 0)) + (int)(scaled_radius * sin(i * factor));
    }

    if (SDL_RenderDrawPoints(renderer, points, 360) != 0)
    {
        LOG("Cannot draw circle. SDL_RenderDrawPoints error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawText(const char* text, int posx, int posy, int w, int h) const
{
    SDL_Color color = { 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    float textureMultiplier = GlobalSettings::GetInstance().GetTextureMultiplier();

    SDL_Rect dstrect = {
        posx,
        posy,
        (int)(w * textureMultiplier),
        (int)(h * textureMultiplier)
    };

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    return true;
}

void Render::DrawTextureEx(SDL_Texture* texture, int x, int y, SDL_Rect* section, int pivotX, int pivotY, double angle)
{
    if (texture == nullptr) return;

    int scale = 1;
    SDL_Rect rect;

    rect.x = (int)(camera.x) + x;
    rect.y = (int)(camera.y) + y;

    if (section != NULL)
    {
        rect.w = section->w;
        rect.h = section->h;
    }
    else
    {
        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    }

    rect.w *= scale;
    rect.h *= scale;

    SDL_Point pivot;
    pivot.x = pivotX * scale;
    pivot.y = pivotY * scale;

    SDL_RenderCopyEx(renderer, texture, section, &rect, angle, &pivot, SDL_FLIP_NONE);
}
