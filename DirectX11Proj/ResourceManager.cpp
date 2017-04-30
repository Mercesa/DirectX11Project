#include "ResourceManager.h"

#include <string>
#include <iostream>
#include <memory.h>

#include "modelclass.h"
#include "ModelData.h"
#include "d3dTexture.h"

#include "easylogging++.h"
ResourceManager::ResourceManager()
{
	mpModelLoader = std::make_unique<ModelLoader>();
}

void ResourceManager::Shutdown()
{
	mLoadedModels.clear();
	mLoadedTextures.clear();
}

void LoadTexturesFromMaterialData(const MeshData& aData)
{

}

// TODO, create a function for model loader which just asks for the next model, this removes it from the model list
std::vector<ModelClass*> ResourceManager::LoadModels(std::string aFilePath)
{
	mpModelLoader->LoadModel(aFilePath.c_str());
	std::vector<ModelClass*> tModels;

	std::unique_ptr<ModelClass> tpModelClass = nullptr;
	std::unique_ptr<d3dTexture> tpTextureClass = nullptr;

	const std::vector<MeshData>& tMeshes = mpModelLoader->GetMeshesToBeProcessed();

	for (unsigned int i = 0; i < tMeshes.size(); ++i)
	{
		const MeshData& tData = tMeshes[i];

		tpModelClass = std::make_unique<ModelClass>();
		tpModelClass->Initialize(this->mpDevice, mpModelLoader->GetMeshesToBeProcessed()[i]);

		tpTextureClass = std::make_unique<d3dTexture>();

		// Convert string texture filepath to wstring
	
		std::wstring wString = std::wstring(tData.specularData.filepath.begin(), tData.specularData.filepath.end());
		const WCHAR* result = wString.c_str();

		// if the single texture could not initialize
		if (!tpTextureClass->Initialize(mpDevice, result))
		{
			LOG(WARNING) << "ModelLoading: Texture could not initialize " << tData.specularData.filepath;
		}
		else
		{
			tpModelClass->mpTexture = tpTextureClass.get();
			tpModelClass->mHastexture = tData.specularData.isValid;
			mLoadedTextures.push_back(std::move(tpTextureClass));
		}

		tModels.push_back(tpModelClass.get());
		mLoadedModels.push_back(std::move(tpModelClass));
	}

	mpModelLoader->ClearProcessedMeshes();

	LOG(INFO) << "ModelLoading: Finished loading model " << aFilePath;
	return tModels;
}

