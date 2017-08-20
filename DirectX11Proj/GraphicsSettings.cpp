#include "GraphicsSettings.h"


uint32_t GraphicsSettings::gCurrentScreenWidth = 1280;
uint32_t GraphicsSettings::gCurrentScreenHeight = 720;

bool GraphicsSettings::gIsApplicationFullScreen = false;
bool GraphicsSettings::gIsVsyncEnabled = true;

bool GraphicsSettings::gShowDebugMenuBar = false;
bool GraphicsSettings::gCollectProfileData = false;

GraphicsSettings::GraphicsSettings()
{
}


GraphicsSettings::~GraphicsSettings()
{
}
