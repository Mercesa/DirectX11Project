#pragma once

#include <d3d11.h>

#include <vector>
#include <memory>
#include <string>

#include "ModelLoader.h"
class ModelClass;


class ResourceManager
{
public:
	friend class GraphicsClass;
	
	ResourceManager(ResourceManager const&) = delete;
	void operator=(ResourceManager const&)	= delete;
	

	static ResourceManager& getInstance()
	{
		static ResourceManager instance;

		return instance;
	}


	std::vector<ModelClass*> LoadModels(std::string aFilePath);

private:
	ResourceManager();
	
	std::unique_ptr<ModelLoader> mpModelLoader;
	ID3D11Device* mpDevice;

	std::vector<std::shared_ptr<ModelClass>> mLoadedModels;
	// Load models as one
	// Load models as a list
};

