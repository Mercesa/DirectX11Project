#pragma once

#include <d3d11.h>

#include <vector>
#include <memory>
#include <string>

#include "ModelLoader.h"
#include "d3dTexture.h"

class ModelClass;
class d3dMaterial;
struct TextureData;

class ResourceManager
{
public:
	friend class GraphicsClass;
	
	ResourceManager(ResourceManager const&) = delete;
	void operator=(ResourceManager const&)	= delete;

	void Shutdown();

	static ResourceManager& GetInstance()
	{
		static ResourceManager instance;

		return instance;
	}

	std::vector<ModelClass*> LoadModels(std::string aFilePath);

private:
	ResourceManager();
	
	std::unique_ptr<ModelLoader> mpModelLoader;
	ID3D11Device* mpDevice;

	// Turn these into maps at one point
	std::vector<std::unique_ptr<ModelClass>> mLoadedModels;
	std::vector<std::unique_ptr<d3dTexture>> mLoadedTextures;

	std::unique_ptr<d3dMaterial> LoadTexturesFromMaterial(const MeshData& aMeshData);
	std::unique_ptr<d3dTexture> LoadTexture(TextureData aData);

	// Load models as one
	// Load models as a list
};

