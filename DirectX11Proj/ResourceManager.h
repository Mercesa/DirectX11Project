#pragma once

#include <d3d11_1.h>

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <D3DX11tex.h>

#include "ModelLoader.h"
#include "d3d11HelperFile.h"
#include "GraphicsStructures.h"

class ModelClass;
struct RawTextureData;

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

	std::vector<ModelID> LoadModels(std::string aFilePath);

	ID3D11Device* mpDevice;

	Model* const GetModelByID(const ModelID& aID) const;
	Texture* const GetTextureByID(const TexID& aID) const;

private:
	ResourceManager();
	

	// Turn these into maps at one point
	//std::vector<std::unique_ptr<Model>> mLoadedModels;

	std::unordered_map<std::string, std::vector<ModelID>> stringModelMap;
	std::vector<Model*> modelData;

	std::unordered_map<std::string, TexID> stringTextureMap;
	std::vector<Texture*> mLoadedTextures;


	d3dMaterial* LoadTexturesFromMaterial(const RawMeshData& aMeshData);
	Texture* LoadTexture(RawTextureData aData);

	// Load models as one
	// Load models as a list
};

