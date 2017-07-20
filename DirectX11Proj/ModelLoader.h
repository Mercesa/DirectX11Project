#pragma once

#include <vector>

#include "GraphicsStructures.h"
struct aiNode;
struct aiScene;
struct aiMesh;


struct RawMeshData;

class ModelLoader
{
public:
	static std::vector<RawMeshData> LoadModel(const char* aFilePath);
	

private:
	ModelLoader();
	~ModelLoader();

	static void ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene, std::vector<RawMeshData>& aData);
	static RawMeshData ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene);

	// has a vector of meshes that are done processing 
};
