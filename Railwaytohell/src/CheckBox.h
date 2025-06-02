#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class CheckBox : public GuiControl
{
public:
    CheckBox(int id, SDL_Rect bounds, const char* text);
    virtual ~CheckBox();

    bool Start() override;
    bool Update(float dt) override;
    bool PostUpdate() override;

    void SetTextures(SDL_Texture* normal, SDL_Texture* pressed);

    void SetState(GuiControlState newState) {
        state = newState;
    }

    void SetChecked(bool newState) {
        checked = newState;
        state = checked ? GuiControlState::PRESSED : GuiControlState::NORMAL;
    }

    bool IsChecked() const { return checked; }

private:
    bool checked = false;
    SDL_Texture* normalTexture = nullptr;
    SDL_Texture* pressedTexture = nullptr;
    int clickFx = -1;
};