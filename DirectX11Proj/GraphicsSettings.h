#pragma once
#include <stdint.h>

struct GraphicsSettings
{
	static uint32_t gCurrentScreenWidth;
	static uint32_t gCurrentScreenHeight;
	static bool gIsApplicationFullScreen;
	static bool gIsVsyncEnabled;

	static bool gShowDebugWindow;

private:
	GraphicsSettings();
	~GraphicsSettings();
};

