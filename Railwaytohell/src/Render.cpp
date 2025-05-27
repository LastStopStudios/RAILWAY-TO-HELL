#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"

#define VSYNC true
#define TEXTURE_SIZE_MULTIPLIER 1.5f  // Factor para hacer las texturas zon zoom

Render::Render() : Module()
{
	name = "render";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{
}

// Called before render is available
bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	//Load the configuration of the Render module
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

	//initialise the SDL_ttf library
	TTF_Init();

	//load a font into memory
	font = TTF_OpenFont("Assets/Fonts/PanasChill.ttf", 13);

	return ret;
}

//Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
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

// Called before quitting
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
	rect.x = (int)(camera.x * speed) + (int)(x * TEXTURE_SIZE_MULTIPLIER);
	rect.y = (int)(camera.y * speed) + (int)(y * TEXTURE_SIZE_MULTIPLIER);

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
	rect.w = (int)(rect.w * TEXTURE_SIZE_MULTIPLIER);
	rect.h = (int)(rect.h * TEXTURE_SIZE_MULTIPLIER);

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		// Scale the pivot point too
		pivot.x = (int)(pivotX * TEXTURE_SIZE_MULTIPLIER);
		pivot.y = (int)(pivotY * TEXTURE_SIZE_MULTIPLIER);
		p = &pivot;
	}

	if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawTextureWithFlip(SDL_Texture* texture, int x, int y, const SDL_Rect* section,
    float speed, double angle, int pivotX, int pivotY,
    SDL_RendererFlip flip) const
{
    bool ret = true;

    SDL_Rect rect;

    // Apply scaling to both position and texture size
    rect.x = (int)(camera.x * speed) + (int)(x * TEXTURE_SIZE_MULTIPLIER);
    rect.y = (int)(camera.y * speed) + (int)(y * TEXTURE_SIZE_MULTIPLIER);

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
    rect.w = (int)(rect.w * TEXTURE_SIZE_MULTIPLIER);
    rect.h = (int)(rect.h * TEXTURE_SIZE_MULTIPLIER);

    SDL_Point* p = NULL;
    SDL_Point pivot;

    if (pivotX != INT_MAX && pivotY != INT_MAX)
    {
        // Scale the pivot point too
        pivot.x = (int)(pivotX * TEXTURE_SIZE_MULTIPLIER);
        pivot.y = (int)(pivotY * TEXTURE_SIZE_MULTIPLIER);
        p = &pivot;
    }

    if (SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
    {
        LOG("Cannot blit to screen. SDL_RenderCopyEx error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
    bool ret = true;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    SDL_Rect rec;
    if (use_camera)
    {
        // Scale both position and dimensions
        rec.x = (int)(camera.x + rect.x );
        rec.y = (int)(camera.y + rect.y );
        rec.w = (int)(rect.w );
        rec.h = (int)(rect.h );
    } 
    else
    {
        // Scale both position and dimensions
        rec.x = (int)(rect.x );
        rec.y = (int)(rect.y );
        rec.w = (int)(rect.w );
        rec.h = (int)(rect.h );
    }

    int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

    if (result != 0)
    {
        LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
    bool ret = true;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    int result = -1;

    // Scale all position coordinates
    if (use_camera)
    {
        result = SDL_RenderDrawLine(renderer,
            (int)(camera.x + x1 * TEXTURE_SIZE_MULTIPLIER),
            (int)(camera.y + y1 * TEXTURE_SIZE_MULTIPLIER),
            (int)(camera.x + x2 * TEXTURE_SIZE_MULTIPLIER),
            (int)(camera.y + y2 * TEXTURE_SIZE_MULTIPLIER));
    }
    else
    {
        result = SDL_RenderDrawLine(renderer,
            (int)(x1 * TEXTURE_SIZE_MULTIPLIER),
            (int)(y1 * TEXTURE_SIZE_MULTIPLIER),
            (int)(x2 * TEXTURE_SIZE_MULTIPLIER),
            (int)(y2 * TEXTURE_SIZE_MULTIPLIER));
    }

    if (result != 0)
    {
        LOG("Cannot draw line to screen. SDL_RenderDrawLine error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
    bool ret = true;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    int result = -1;
    SDL_Point points[360];

    float factor = (float)M_PI / 180.0f;

    // Scale both position and radius
    int scaled_x = (int)(x * TEXTURE_SIZE_MULTIPLIER);
    int scaled_y = (int)(y * TEXTURE_SIZE_MULTIPLIER);
    int scaled_radius = (int)(radius * TEXTURE_SIZE_MULTIPLIER);

    for (int i = 0; i < 360; ++i)
    {
        points[i].x = (int)(scaled_x + (use_camera ? camera.x : 0)) + (int)(scaled_radius * cos(i * factor));
        points[i].y = (int)(scaled_y + (use_camera ? camera.y : 0)) + (int)(scaled_radius * sin(i * factor));
    }

    result = SDL_RenderDrawPoints(renderer, points, 360);

    if (result != 0)
    {
        LOG("Cannot draw circle to screen. SDL_RenderDrawPoints error: %s", SDL_GetError());
        ret = false;
    }

    return ret;
}

bool Render::DrawText(const char* text, int posx, int posy, int w, int h) const
{
    SDL_Color color = { 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);

    // Scale only the size, keeping the original position
    SDL_Rect dstrect = {
        posx,
        posy,
        (int)(w * TEXTURE_SIZE_MULTIPLIER),
        (int)(h * TEXTURE_SIZE_MULTIPLIER)
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

    // Define the pivot point for rotation
    SDL_Point pivot;
    pivot.x = pivotX * scale;
    pivot.y = pivotY * scale;

    // Render with rotation
    SDL_RenderCopyEx(renderer, texture, section, &rect, angle, &pivot, SDL_FLIP_NONE);
}
