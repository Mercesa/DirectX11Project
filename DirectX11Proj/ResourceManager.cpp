#include "ResourceManager.h"

#include <string>
#include <iostream>
#include <memory.h>

#include "d3dTexture.h"
#include "easylogging++.h"
#include "GraphicsStructures.h"
ResourceManager::ResourceManager()
{
}

void ResourceManager::Shutdown()
{

	for (int i = 0; i < modelData.size(); ++i)
	{
		ReleaseModel(modelData[i]);
		delete modelData[i];
		modelData[i] = nullptr;
	}

	for (int i = 0; i < mLoadedTextures.size(); ++i)
	{
		delete mLoadedTextures[i];
		mLoadedTextures[i] = nullptr;
		//delete modelData[i];
	}
	
	//modelData.clear();
	//mLoadedTextures.clear();
	
	LOG(INFO) << "ResourceManager cleaned up";
}

Model* const ResourceManager::GetModelByID(const ModelID& aID) const
{
	if (aID.GetID() >= modelData.size())
	{
		LOG(ERROR) << "GetModelByID failed, supplied ID out of bounds";
		return nullptr;
	}
	
	return modelData[aID.GetID()];
}

d3dTexture* const ResourceManager::GetTextureByID(const TexID& aID) const
{
	if (aID.GetID() >= mLoadedTextures.size())
	{
		LOG(ERROR) << "GetModelByID failed, supplied ID out of bounds";
		return nullptr;
	}

	return mLoadedTextures[aID.GetID()];
}


d3dTexture* ResourceManager::LoadTexture(RawTextureData aData)
{
	// Convert string texture filepath to wstring
	std::wstring wString = std::wstring(aData.filepath.begin(), aData.filepath.end());
	const WCHAR* result = wString.c_str();

	auto tpTextureClass = new d3dTexture();
	assert(tpTextureClass);
	// if the single texture could not initialize
	if (!tpTextureClass->Initialize(mpDevice, result))
	{
		LOG(WARNING) << "ModelLoading: Texture could not initialize " << aData.filepath;
	}

	return tpTextureClass;
}


d3dMaterial* ResourceManager::LoadTexturesFromMaterial(const RawMeshData& aMeshData)
{
	d3dMaterial* tpMat = new d3dMaterial();

	assert(tpMat);

	
	if (this->stringTextureMap.find(aMeshData.diffuseData.filepath) != stringTextureMap.end())
	{
		tpMat->mpDiffuse = stringTextureMap[aMeshData.diffuseData.filepath];
	}
	else
	{
		tpMat->mpDiffuse = TexID(mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTexture(aMeshData.diffuseData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.diffuseData.filepath, tpMat->mpDiffuse));
	}

	if (this->stringTextureMap.find(aMeshData.specularData.filepath) != stringTextureMap.end())
	{
		tpMat->mpSpecular = stringTextureMap[aMeshData.specularData.filepath];
	}
	else
	{
		tpMat->mpSpecular = TexID(mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTexture(aMeshData.specularData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.specularData.filepath, tpMat->mpSpecular));
	}

	if (this->stringTextureMap.find(aMeshData.normalData.filepath) != stringTextureMap.end())
	{
		tpMat->mpNormal = stringTextureMap[aMeshData.normalData.filepath];
	}
	else
	{
		tpMat->mpNormal = TexID(mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTexture(aMeshData.normalData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.normalData.filepath, tpMat->mpNormal));
	}

	return tpMat;
	//mLoadedTextures.push_back(std::move(tpTextureClass));
}

// TODO, create a function for model loader which just asks for the next model, this removes it from the model list
std::vector<ModelID> ResourceManager::LoadModels(std::string aFilePath)
{

	// Early out
	if (this->stringModelMap.find(aFilePath) != stringModelMap.end())
	{
		return stringModelMap[aFilePath];
	}

	std::vector<ModelID> tModels;
	const std::vector<RawMeshData>& tMeshes = ModelLoader::LoadModel(aFilePath.c_str());

	for (unsigned int i = 0; i < tMeshes.size(); ++i)
	{
		const RawMeshData& tData = tMeshes[i];

		Model* tMod = CreateSimpleModelFromRawData(mpDevice, tData);


		tMod->material = LoadTexturesFromMaterial(tData);
		assert(tMod->material != nullptr);
		ModelID id = ModelID(modelData.size());
		tModels.push_back(id);

		modelData.push_back(tMod);
	}

	stringModelMap[aFilePath] = tModels;
	LOG(INFO) << "ModelLoading: Finished loading model " << aFilePath;
	return tModels;
}

