#pragma once


#include <DirectXMath.h>
#include <vector>

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

};