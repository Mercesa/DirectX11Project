#include "GraphicsSettings.h"


uint32_t GraphicsSettings::gCurrentScreenWidth = 1920;
uint32_t GraphicsSettings::gCurrentScreenHeight = 1080;

bool GraphicsSettings::gIsApplicationFullScreen = false;
bool GraphicsSettings::gIsVsyncEnabled = true;

bool GraphicsSettings::gShowDebugWindow = false;
bool GraphicsSettings::gCollectProfileData = false;

GraphicsSettings::GraphicsSettings()
{
}


GraphicsSettings::~GraphicsSettings()
{
}
