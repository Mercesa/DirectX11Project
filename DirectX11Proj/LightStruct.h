#pragma once

#include <DirectXMath.h>

using namespace DirectX;

enum eLightTypes
{
	ePOINT, eDIRECTIONAL, eSPOT
};

struct Light
{
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float pad0;
	
	XMFLOAT3 diffuseColor = XMFLOAT3(0.5f, 0.5f, 0.5f);
	float pad1;
	
	XMFLOAT3 specularColor = XMFLOAT3(1.0f, 0.0f, 1.0f);
	int typeOfLight;
};
