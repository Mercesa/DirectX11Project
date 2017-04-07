#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#include <vector>


struct VertexData
{
	XMFLOAT3 position;
	XMFLOAT2 texCoords;
	XMFLOAT3 normal;
	XMFLOAT3 bitangent;
	XMFLOAT3 tangent;
};

struct TextureData
{
	std::string filepath;
};

struct MeshData
{
	// Vertices, indices
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;

	std::string textureFilePath;

};