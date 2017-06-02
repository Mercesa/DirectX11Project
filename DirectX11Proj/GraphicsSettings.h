#pragma once
#include <stdint.h>

class GraphicsSettings
{
public:

	static uint32_t gCurrentScreenWidth;
	static uint32_t gCurrentScreenHeight;
	static bool gIsApplicationFullScreen;
	static bool gIsVsyncEnabled;

	static bool gShowDebugWindow;
private:
	GraphicsSettings();
	~GraphicsSettings();
};

