#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiControlSlider : public GuiControl
{
public:
    GuiControlSlider(int id, SDL_Rect bounds, const char* text, int minValue, int maxValue);
    virtual ~GuiControlSlider();

    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;

    void SetValue(int newValue);
    int GetValue() const;
    void SetState(GuiControlState newState) {
        state = newState;
    }

    void SetTextures(SDL_Texture* baseTexture, SDL_Texture* handleTexture);
    void SetHandleSize(int width, int height);

    void UpdateHandlePosition();

private:
    int value;
    int minValue;
    int maxValue;
    SDL_Texture* baseTexture = nullptr;  
    SDL_Texture* handleTexture = nullptr; 
    SDL_Rect handleBounds; // Handle area
    bool dragging = false;                

    int clickFx = -1;
};