#include "CheckBox.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Input.h"

CheckBox::CheckBox(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::CHECKBOX, id)
{
    this->bounds = bounds;
    this->text = text;
}

CheckBox::~CheckBox()
{
}

bool CheckBox::Start()
{
    clickFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/button-pressed.wav");
    return true;
}

void CheckBox::SetTextures(SDL_Texture* normal, SDL_Texture* pressed)
{
    normalTexture = normal;
    pressedTexture = pressed;
}

bool CheckBox::Update(float dt)
{
    if (state == GuiControlState::DISABLED) return false;

    Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();
    bool mouseHover = (mousePos.getX() > bounds.x &&
        mousePos.getX() < bounds.x + bounds.w &&
        mousePos.getY() > bounds.y &&
        mousePos.getY() < bounds.y + bounds.h);

    if (mouseHover && Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
    {
        checked = !checked;
        Engine::GetInstance().audio->PlayFx(clickFx);
        NotifyObserver();
        
        state = checked ? GuiControlState::PRESSED : GuiControlState::NORMAL;
    }

    return false;
}

bool CheckBox::PostUpdate()
{
    if (state == GuiControlState::DISABLED) return true;

    if (state == GuiControlState::PRESSED && pressedTexture) {
        Engine::GetInstance().render->DrawTextureForCheckBox(pressedTexture, bounds.x, bounds.y, nullptr, 0.0f);
    }
    else if (normalTexture) {
        Engine::GetInstance().render->DrawTextureForCheckBox(normalTexture, bounds.x, bounds.y, nullptr, 0.0f);
    }

    return true;
}