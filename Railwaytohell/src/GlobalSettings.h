#pragma once

class GlobalSettings
{
public:
    // Singleton instance
    static GlobalSettings& GetInstance();

    // Texture size multiplier - accessible and modifiable from anywhere
    float GetTextureMultiplier() const { return textureMultiplier; }
    void SetTextureMultiplier(float multiplier) { textureMultiplier = multiplier; }

private:
    GlobalSettings() : textureMultiplier(1.5f) {} // Default value
    ~GlobalSettings() = default;

    // Delete copy constructor and assignment operator
    GlobalSettings(const GlobalSettings&) = delete;
    GlobalSettings& operator=(const GlobalSettings&) = delete;

    float textureMultiplier;
};