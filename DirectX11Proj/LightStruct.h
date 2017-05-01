#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Light
{
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float pad0;
	XMFLOAT3 colour = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float pad1;
};
