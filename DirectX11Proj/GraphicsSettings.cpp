#include "GraphicsSettings.h"


uint32_t GraphicsSettings::gCurrentScreenWidth = 1024;
uint32_t GraphicsSettings::gCurrentScreenHeight = 768;

bool GraphicsSettings::gIsApplicationFullScreen = false;
bool GraphicsSettings::gIsVsyncEnabled = true;

bool GraphicsSettings::gShowDebugWindow = false;

GraphicsSettings::GraphicsSettings()
{
}


GraphicsSettings::~GraphicsSettings()
{
}
