#pragma once

#include <d3d11.h>	

class d3dDXGIManager
{
public:
	d3dDXGIManager();
	~d3dDXGIManager();

	bool Create(int aWidth, int aHeight, int &numerator, int &denominator);

	int m_videoCardMemory;
	char m_videoCardDescription[128];
};

