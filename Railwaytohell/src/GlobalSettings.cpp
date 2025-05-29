#include "GlobalSettings.h"

GlobalSettings& GlobalSettings::GetInstance()
{
    static GlobalSettings instance;
    return instance;
}
