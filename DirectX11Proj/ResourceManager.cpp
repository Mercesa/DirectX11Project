#include "ResourceManager.h"

#include <string>
#include <iostream>
#include <memory.h>

#include "modelclass.h"
#include "d3dTexture.h"
#include "d3dMaterial.h"
#include "easylogging++.h"
ResourceManager::ResourceManager()
{
}

void ResourceManager::Shutdown()
{
	mLoadedModels.clear();
	mLoadedTextures.clear();
}


std::unique_ptr<d3dTexture> ResourceManager::LoadTexture(RawTextureData aData)
{
	// Convert string texture filepath to wstring
	std::wstring wString = std::wstring(aData.filepath.begin(), aData.filepath.end());
	const WCHAR* result = wString.c_str();

	auto tpTextureClass = std::make_unique<d3dTexture>();
	assert(tpTextureClass);
	// if the single texture could not initialize
	if (!tpTextureClass->Initialize(mpDevice, result))
	{
		LOG(WARNING) << "ModelLoading: Texture could not initialize " << aData.filepath;
	}

	return std::move(tpTextureClass);
}

std::unique_ptr<d3dMaterial> ResourceManager::LoadTexturesFromMaterial(const RawMeshData& aMeshData)
{
	auto tpMat = std::make_unique<d3dMaterial>();

	assert(tpMat);
	tpMat->mpDiffuse = LoadTexture(aMeshData.diffuseData);
	tpMat->mpSpecular = LoadTexture(aMeshData.specularData);
	tpMat->mpNormal = LoadTexture(aMeshData.normalData);

	return tpMat;
	//mLoadedTextures.push_back(std::move(tpTextureClass));
}

// TODO, create a function for model loader which just asks for the next model, this removes it from the model list
std::vector<ModelClass*> ResourceManager::LoadModels(std::string aFilePath)
{

	std::vector<ModelClass*> tModels;

	std::unique_ptr<ModelClass> tpModelClass = nullptr;
	std::unique_ptr<d3dTexture> tpTextureClass = nullptr;

	const std::vector<RawMeshData>& tMeshes = ModelLoader::LoadModel(aFilePath.c_str());

	for (unsigned int i = 0; i < tMeshes.size(); ++i)
	{
		const RawMeshData& tData = tMeshes[i];

		tpModelClass = std::make_unique<ModelClass>();
		tpModelClass->Initialize(this->mpDevice, tData);
		tpModelClass->mMaterial = std::move(LoadTexturesFromMaterial(tData));

		tModels.push_back(tpModelClass.get());
		mLoadedModels.push_back(std::move(tpModelClass));
	}

	LOG(INFO) << "ModelLoading: Finished loading model " << aFilePath;
	return tModels;
}

