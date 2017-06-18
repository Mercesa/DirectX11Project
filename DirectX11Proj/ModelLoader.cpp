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

std::string mDirectory;


void ModelLoader::ClearProcessedMeshes()
{
	this->mMeshesToBeProcessed.clear();
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


const std::vector<RawMeshData>& ModelLoader::GetMeshesToBeProcessed()
{
	return mMeshesToBeProcessed;
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
}


// Process mesh
void ModelLoader::ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene)
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


	this->mMeshesToBeProcessed.push_back(meshData);
}

// This function will be called recursively if there is more than 1 node in a scene
void ModelLoader::ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene)
{
	// Process meshes in node
	for (unsigned int i = 0; i < a_Node->mNumMeshes; i++)
	{
		ProcessMesh(a_Scene->mMeshes[a_Node->mMeshes[i]], a_Scene);
		
	}

	// Process children of scene recursively
	for (unsigned int j = 0; j < a_Node->mNumChildren; j++)
	{
		ProcessNode((a_Node->mChildren[j]), a_Scene);
	}
}

void ModelLoader::LoadModel(const char* const aFilePath)
{
	Assimp::Importer importer;



	const aiScene* scene;

	scene = importer.ReadFile(aFilePath, aiProcess_GenUVCoords | aiProcess_FlipUVs | aiProcessPreset_TargetRealtime_Fast | aiProcess_CalcTangentSpace);
	assert(scene != nullptr);

	if (!scene)
	{
		printf("%s", importer.GetErrorString());
	}

	// Set directory string and c_string
	std::string aFilePathStr(aFilePath);

	mDirectory = aFilePathStr.substr(0, aFilePathStr.find_last_of("\\"));
	
	ProcessNode(scene->mRootNode, scene);
	//printf("%i", mMeshesToBeProcessed.size());
}