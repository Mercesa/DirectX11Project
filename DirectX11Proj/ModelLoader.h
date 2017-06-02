#pragma once

#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;


struct MeshData;

class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();

	void LoadModel(const char* aFilePath);
	
	const std::vector<MeshData>& GetMeshesToBeProcessed();
	void ClearProcessedMeshes();

private:
	void ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene);
	void ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene);

	// has a vector of meshes that are done processing 
	std::vector<MeshData> mMeshesToBeProcessed;
};
