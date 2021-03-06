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

	std::vector<ModelID> LoadModels(std::string aFilePath, bool aGenerateAABB);

	ID3D11Device* mpDevice;

	Model* const GetModelByID(const ModelID& aID) const;
	Texture* const GetTextureByID(const TexID& aID) const;

	Material* LoadTexturesFromMaterial(const RawMeshData& aMeshData);
	Material* LoadCubeMapTexturesFromMaterial(const RawMeshData& aMeshData);

private:
	ResourceManager();
	
	// Models and corresponding vector with data
	std::unordered_map<std::string, std::vector<ModelID>> stringModelMap;
	std::vector<Model*> modelData;

	// Textures and corresponding vector with data
	std::unordered_map<std::string, TexID> stringTextureMap;
	std::vector<Texture*> mLoadedTextures;

	std::vector<Material*> mLoadedMaterials;

	Texture* LoadTexture(RawTextureData aData);
	Texture* LoadTextureCube(RawTextureData aData);
};

