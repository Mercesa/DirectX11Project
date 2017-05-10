#pragma once

#include "LightStruct.h"

struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	XMFLOAT3 gEyePos;
	float pad0;

	XMMATRIX lightViewMatrix;
	XMMATRIX lightProjectionMatrix;
};

struct MaterialBufferType
{
	int hasDiffuse; // 4 bytes
	int hasSpecular;// 8 bytes
	int hasNormal;	// 12 bytes
	int padding0;	// 16 bytes, 16 bytes aligned :)
};

struct LightBufferType
{
	int amountOfLights;
	int padLB01;
	int padLB02;
	int padLB03;

	Light directionalLight;

	Light arr[16];
};