#pragma once


#include <DirectXMath.h>
#include <vector>

#include "GenericMathValueStructs.h"

#include <glm/common.hpp>
using namespace DirectX;

enum eLightTypes
{
	ePOINT, eDIRECTIONAL, eSPOT
};

struct Light
{
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	float pad0;

	glm::vec3 diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	float pad1;

	glm::vec3 specularColor = glm::vec3(1.0f, 0.0f, 1.0f);
	int typeOfLight;
};


struct VertexData
{
	XMFLOAT3 position;
	XMFLOAT2 texCoords;
	XMFLOAT3 normal;
	XMFLOAT3 bitangent;
	XMFLOAT3 tangent;
};


struct RawTextureData
{
	std::string filepath;
	bool isValid;
};


struct RawMeshData
{
	// Vertices, indices
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;

	RawTextureData diffuseData;
	RawTextureData specularData;
	RawTextureData normalData;

	VEC4f sphericalCollider;
};


