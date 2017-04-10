#include "ResourceManager.h"

#include "modelclass.h"
#include "ModelData.h"

#include "easylogging++.h"

ResourceManager::ResourceManager()
{
	mpModelLoader = std::make_unique<ModelLoader>();
}


// TODO, create a function for model loader which just asks for the next model, this removes it from the model list
std::vector<ModelClass*> ResourceManager::LoadModels(std::string aFilePath)
{
	mpModelLoader->LoadModel(aFilePath.c_str());
	std::vector<ModelClass*> tModels;

	std::shared_ptr<ModelClass> tModelClass = nullptr;
	
	for (unsigned int i = 0; i < mpModelLoader->GetMeshesToBeProcessed().size(); ++i)
	{
		tModelClass = std::make_shared<ModelClass>();
		tModelClass->Initialize(this->mpDevice, mpModelLoader->GetMeshesToBeProcessed()[i]);

		tModels.push_back(tModelClass.get());
		mLoadedModels.push_back(std::move(tModelClass));
	}

	mpModelLoader->ClearProcessedMeshes();

	LOG(INFO) << "ModelLoading: Finished loading model " << aFilePath;
	return tModels;
}

