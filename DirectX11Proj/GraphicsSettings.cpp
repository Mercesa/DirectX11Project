#include "GraphicsSettings.h"


uint32_t GraphicsSettings::gCurrentScreenWidth = 800;
uint32_t GraphicsSettings::gCurrentScreenHeight = 600;

bool GraphicsSettings::gIsApplicationFullScreen = false;
bool GraphicsSettings::gIsVsyncEnabled = true;

GraphicsSettings::GraphicsSettings()
{
}


GraphicsSettings::~GraphicsSettings()
{
}
