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
	this->mLoadedModels.clear();
	this->mLoadedTextures.clear();
}


// TODO, create a function for model loader which just asks for the next model, this removes it from the model list
std::vector<ModelClass*> ResourceManager::LoadModels(std::string aFilePath)
{
	mpModelLoader->LoadModel(aFilePath.c_str());
	std::vector<ModelClass*> tModels;

	std::unique_ptr<ModelClass> tModelClass = nullptr;
	std::unique_ptr<d3dTexture> tTextureClass = nullptr;

	const std::vector<MeshData>& tMeshes = mpModelLoader->GetMeshesToBeProcessed();

	for (unsigned int i = 0; i < tMeshes.size(); ++i)
	{
		const MeshData& tData = tMeshes[i];

		tModelClass = std::make_unique<ModelClass>();
		tModelClass->Initialize(this->mpDevice, mpModelLoader->GetMeshesToBeProcessed()[i]);

		tTextureClass = std::make_unique<d3dTexture>();

		// Convert string texture filepath to wstring
	
		std::wstring wString = std::wstring(tData.textureFilePath.begin(), tData.textureFilePath.end());
		const WCHAR* result = wString.c_str();

		// if the single textuer could not initialize
		if (!tTextureClass->Initialize(mpDevice, result))
		{
			LOG(WARNING) << "ModelLoading: Finished loading model " << aFilePath;
		}

		else
		{
			tModelClass->mpTexture = tTextureClass.get();
			tModelClass->mHastexture = true;
			mLoadedTextures.push_back(std::move(tTextureClass));

		}

		tModels.push_back(tModelClass.get());
		mLoadedModels.push_back(std::move(tModelClass));
	}

	mpModelLoader->ClearProcessedMeshes();

	LOG(INFO) << "ModelLoading: Finished loading model " << aFilePath;
	return tModels;
}

