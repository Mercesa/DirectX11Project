#pragma once

#include "GraphicsStructures.h"
#include "GenericMathValueStructs.h"
#include <glm\common.hpp>

struct cbMatrixBuffer
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewMatrixInversed;
	glm::mat4 projectionMatrixInverse;
	glm::mat4 projViewMatrix;
	glm::mat4 prevProjViewMatrix;

	float gEyePosX;
	float gEyePosY;
	float gEyePosZ;
	float pad0;
};

struct cbLightMatrix
{
	float shadowMapWidth;
	float shadowMapheight;
	float pad0;
	float pad1;
	glm::mat4 lightViewMatrix;
	glm::mat4 lightProjectionMatrix;
	glm::mat4 lightProjectionViewMatrix;

	VEC3f kernelSamples[64];
	VEC4f pad[16];
};

__declspec(align(16))
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

__declspec(align(16))
struct cbPerObject
{
	glm::mat4 worldMatrix;
	glm::mat4 prevWorldMatrix;
};

__declspec(align(16))
struct cbBlurParameters
{
	int blurHorizontal;
	int pad0;
	int pad1;
	int pad2;
};

//__declspec(align(16))
struct cbGenericAttributesBuffer
{
	float screenWidth;
	float screenHeight;
	float nearPlaneDistance;
	float farPlaneDistance;

	float totalApplicationTime;
	float deltaTime;
	float framerate;
};