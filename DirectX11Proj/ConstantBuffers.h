#pragma once

#include "GraphicsStructures.h"
#include "GenericMathValueStructs.h"

struct cbMatrixBuffer
{
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX viewMatrixInversed;

	float gEyePosX;
	float gEyePosY;
	float gEyePosZ;
	float pad0;
};

struct cbLightMatrix
{
	XMMATRIX lightViewMatrix;
	XMMATRIX lightProjectionMatrix;
	VEC3f kernelSamples[64];
	VEC4f padding0[16];
};

struct cbMaterial
{
	int hasDiffuse; // 4 bytes
	int hasSpecular;// 8 bytes
	int hasNormal;	// 12 bytes
	int padding0;	// 16 bytes, 16 bytes aligned :)
};

struct cbLights
{
	int amountOfLights;
	int padLB01;
	int padLB02;
	int padLB03;

	Light directionalLight;

	Light arr[16];
};

struct cbPerObject
{
	XMMATRIX worldMatrix;
};