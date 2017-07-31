#include "ModelLoader.h"

#include <Importer.hpp>
#include <Exporter.hpp>
#include <postprocess.h>
#include <scene.h>
#include <iostream>


using namespace Assimp;

ModelLoader::ModelLoader()
{
}


ModelLoader::~ModelLoader()
{
}



void ProcessVertices(aiMesh* const a_Mesh, std::vector<VertexData>& a_Vertices)
{
	// Loop through vertices
	for (unsigned int i = 0; i < a_Mesh->mNumVertices; i++)
	{
		aiVector3D &vertice = a_Mesh->mVertices[i];
		aiVector3D &tang = a_Mesh->mTangents[i];
		aiVector3D &bTang = a_Mesh->mBitangents[i];

		VertexData vert;

		// Set position
		vert.position.x = vertice.x;
		vert.position.y = vertice.y;
		vert.position.z = vertice.z;

		if (a_Mesh->HasTangentsAndBitangents())
		{
			// Set Tang
			vert.tangent.x = tang.x;
			vert.tangent.y = tang.y;
			vert.tangent.z = tang.z;

			// Set BiTang
			vert.bitangent.x = bTang.x;
			vert.bitangent.y = bTang.y;
			vert.bitangent.z = bTang.z;
		}

		// Set normals
		if (a_Mesh->HasNormals())
		{
			vert.normal.x = a_Mesh->mNormals[i].x;
			vert.normal.y = a_Mesh->mNormals[i].y;
			vert.normal.z = a_Mesh->mNormals[i].z;
		}

		// Set tex coords
		if (a_Mesh->HasTextureCoords(0))
		{
			//glm::vec2 tex;
			vert.texCoords.x = a_Mesh->mTextureCoords[0][i].x;
			vert.texCoords.y = a_Mesh->mTextureCoords[0][i].y;
		}

		// Put it in vec
		a_Vertices.push_back(vert);
	}
}


// Process indices
void ProcessIndices(aiMesh* const a_Mesh, std::vector<uint32_t>& a_Indices)
{
	if (a_Mesh->HasFaces())
	{
		for (unsigned int i = 0; i < a_Mesh->mNumFaces; i++)
		{
			for (unsigned int j = 0; j < a_Mesh->mFaces[i].mNumIndices; j++)
			{
				a_Indices.push_back(a_Mesh->mFaces[i].mIndices[j]);
			}
		}
	}
}

bool DoesMaterialHaveTextures(aiMaterial* const aMat, aiTextureType a_Type)
{
	// Check the amount of textures of a specific type
	if ((aMat->GetTextureCount(a_Type)) <= 0)
	{
		return false;
	}

	return true;
}

std::string GetTextureLocation(aiMaterial* const a_Mat, aiTextureType a_Type)
{
	aiString str;
	a_Mat->GetTexture(a_Type, 0, &str);

	std::string stString = std::string(str.C_Str());


	// Add DDS to the file extension
	std::size_t dotPos = stString.find_last_of(".");
	stString.erase(dotPos, stString.size());

	stString.append(".DDS");

	return stString;
}


// Goes through the material
void ProcessMaterial(aiMesh* a_Mesh, const aiScene* a_Scene, RawMeshData& aMeshdata)
{
	// If we have a material
	if (a_Mesh->mMaterialIndex > 0)
	{
		aiMaterial* const material = a_Scene->mMaterials[a_Mesh->mMaterialIndex];

		aMeshdata.diffuseData.isValid = DoesMaterialHaveTextures(material, aiTextureType_DIFFUSE);
		aMeshdata.specularData.isValid = DoesMaterialHaveTextures(material, aiTextureType_SPECULAR);
		aMeshdata.normalData.isValid = DoesMaterialHaveTextures(material, aiTextureType_HEIGHT);

		if (aMeshdata.diffuseData.isValid)
		{
			aMeshdata.diffuseData.filepath = GetTextureLocation(material, aiTextureType_DIFFUSE);
		}

		if (aMeshdata.specularData.isValid)
		{
			aMeshdata.specularData.filepath = GetTextureLocation(material, aiTextureType_SPECULAR);
		}

		if (aMeshdata.normalData.isValid)
		{
			aMeshdata.normalData.filepath = GetTextureLocation(material, aiTextureType_HEIGHT);
			
		}
	}

	// No material present with object
	//else
	//{
	//	aMeshdata.diffuseData.isValid = false;
	//	aMeshdata.normalData.isValid = false;
	//	aMeshdata.specularData.isValid = false;
	//
	//	aMeshdata.diffuseData.filepath = "";
	//	aMeshdata.specularData.filepath = "";
	//	aMeshdata.normalData.filepath = "";
	//
	//}
}


// Process mesh
RawMeshData ModelLoader::ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene)
{
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;

	// Process vertices & indices
	ProcessVertices(a_Mesh, vertices);
	ProcessIndices(a_Mesh, indices);

	RawMeshData meshData;

	ProcessMaterial(a_Mesh, a_Scene, meshData);


	meshData.vertices = vertices;
	meshData.indices = indices;


	return meshData;
}

// This function will be called recursively if there is more than 1 node in a scene
void ModelLoader::ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene, std::vector<RawMeshData>& aData)
{
	// Process meshes in node
	for (unsigned int i = 0; i < a_Node->mNumMeshes; i++)
	{
		aData.push_back(ProcessMesh(a_Scene->mMeshes[a_Node->mMeshes[i]], a_Scene));
	}

	// Process children of scene recursively
	for (unsigned int j = 0; j < a_Node->mNumChildren; j++)
	{
		ProcessNode((a_Node->mChildren[j]), a_Scene, aData);
	}
}


#include <fstream>
#include "easylogging++.h"

bool is_file_exist(const char *fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

// LoadModel implicitly exports a file to assbin right now, and next time it will load the assbin version if possible
std::vector<RawMeshData> ModelLoader::LoadModel(const char* const aFilePath)
{
	bool doesAssbinVersionExist = false;
	// Set directory string and c_string
	std::string assbinString(aFilePath);


	std::size_t dotPos = assbinString.find_last_of(".");
	assbinString.erase(dotPos, assbinString.size());

	assbinString.append(".assbin");

	doesAssbinVersionExist = is_file_exist(assbinString.c_str());


	const aiScene* scene;
	Assimp::Importer importer;

	std::string filePathToBeLoaded = aFilePath;
	
	// If .assbin version of file exist, load that one
	if (doesAssbinVersionExist)
	{
		LOG(INFO) << "ModelLoader Assbin version exists, loading";
		filePathToBeLoaded = assbinString;
		scene = importer.ReadFile(filePathToBeLoaded.c_str(), 0);
	}

	else
	{
		LOG(INFO) << "ModelLoader Assbin version created of asset";
		Exporter exporter;
		scene = importer.ReadFile(filePathToBeLoaded.c_str(), aiProcess_FlipUVs | aiProcessPreset_TargetRealtime_Fast | aiProcess_CalcTangentSpace);
		exporter.Export(scene, "assbin", assbinString, 0);
	}


	assert(scene != nullptr);
	if (!scene)
	{
		printf("%s", importer.GetErrorString());
	}
	

	std::vector<RawMeshData> tData;
	ProcessNode(scene->mRootNode, scene, tData);

	return tData;
}