#include "GuiControlSlider.h"

#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Input.h"

GuiControlSlider::GuiControlSlider(int id, SDL_Rect bounds, const char* text, int minValue, int maxValue) : GuiControl(GuiControlType::SLIDER, id)
{
    this->bounds = bounds;
    this->text = text;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->value = (minValue + maxValue) / 2;

    handleBounds.w = 20;
    handleBounds.h = 20;

    handleBounds.y = bounds.y + (bounds.h - handleBounds.h) / 2;

    UpdateHandlePosition();
}

GuiControlSlider::~GuiControlSlider()
{
}

bool GuiControlSlider::Start()
{
    clickFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/button-pressed.wav");
    return true;
}

void GuiControlSlider::SetTextures(SDL_Texture* baseTexture, SDL_Texture* handleTexture)
{
    this->baseTexture = baseTexture;
    this->handleTexture = handleTexture;
}

void GuiControlSlider::SetHandleSize(int width, int height)
{
    handleBounds.w = width;
    handleBounds.h = height;
    handleBounds.y = bounds.y + (bounds.h - handleBounds.h) / 2;
    UpdateHandlePosition();
}

void GuiControlSlider::SetValue(int newValue)
{
    if (newValue < minValue) newValue = minValue;
    if (newValue > maxValue) newValue = maxValue;
    value = newValue;
    UpdateHandlePosition();
}

int GuiControlSlider::GetValue() const
{
    return value;
}

void GuiControlSlider::UpdateHandlePosition()
{
    int range = maxValue - minValue;
    if (range == 0) range = 1; 

    int position = bounds.x + (bounds.w * (value - minValue)) / range;
    handleBounds.x = position - (handleBounds.w / 2);
}

bool GuiControlSlider::Update(float dt)
{
    if (state == GuiControlState::DISABLED) return false;

    Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

    // Check if the mouse is within the bounds of the slider
    bool mouseOverHandle = (mousePos.getX() > handleBounds.x &&
        mousePos.getX() < handleBounds.x + handleBounds.w &&
        mousePos.getY() > handleBounds.y &&
        mousePos.getY() < handleBounds.y + handleBounds.h);

    //  Check if the mouse is within the bounds of the slider base
    bool mouseOverBase = (mousePos.getX() > bounds.x &&
        mousePos.getX() < bounds.x + bounds.w &&
        mousePos.getY() > bounds.y &&
        mousePos.getY() < bounds.y + bounds.h);

    // Handling mouse hover state
    if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
    {
        if (mouseOverHandle)
        {
            Engine::GetInstance().audio->PlayFx(clickFx);
            // Start dragging if the handle is clicked
            dragging = true;
            state = GuiControlState::PRESSED;
        }
        else if (mouseOverBase)
        {
            Engine::GetInstance().audio->PlayFx(clickFx);
            int relativeX = mousePos.getX() - bounds.x;

            int range = maxValue - minValue;
            if (range < 1) range = 1; 

            if (relativeX < 0) relativeX = 0;
            if (relativeX > bounds.w) relativeX = bounds.w;

            value = minValue + (relativeX * range) / bounds.w;

            UpdateHandlePosition();
            dragging = true;
            state = GuiControlState::PRESSED;
            NotifyObserver();
        }
    }

    // Handle dragging state
    if (dragging && Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
    {
        int relativeX = mousePos.getX() - bounds.x;

        int range = maxValue - minValue;
        if (range < 1) range = 1; 

        if (relativeX < 0) relativeX = 0;
        if (relativeX > bounds.w) relativeX = bounds.w;

        value = minValue + (relativeX * range) / bounds.w;

        UpdateHandlePosition();
        state = GuiControlState::PRESSED;
        NotifyObserver();
    }

    // Handle mouse button release
    if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
    {
        dragging = false;

        if (mouseOverHandle || mouseOverBase) {
            state = GuiControlState::FOCUSED;
        }
        else {
            state = GuiControlState::NORMAL;
        }
    }

    // Update mouse visual state
    if (!dragging)
    {
        if (mouseOverHandle || mouseOverBase)
        {
            state = GuiControlState::FOCUSED;
        }
        else
        {
            state = GuiControlState::NORMAL;
        }
    }

    return false;
}

bool GuiControlSlider::PostUpdate()
{
    if (state == GuiControlState::DISABLED) return true;

    // Draw slider base
    if (baseTexture) {
        Engine::GetInstance().render->DrawTextureForCheckBox(baseTexture, bounds.x, bounds.y + (bounds.h / 2) - 9, nullptr, 0.0f);
    }

    // Draw slider handle
    if (handleTexture) {
        Engine::GetInstance().render->DrawTextureForCheckBox(handleTexture, handleBounds.x, handleBounds.y, nullptr, 0.0f);
    }

    return true;
}