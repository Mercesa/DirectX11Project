#include "ResourceManager.h"

#include <string>
#include <iostream>
#include <memory.h>

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
		ReleaseTexture(mLoadedTextures[i]);
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

Texture* const ResourceManager::GetTextureByID(const TexID& aID) const
{
	if (aID.GetID() >= mLoadedTextures.size())
	{
		LOG(ERROR) << "GetModelByID failed, supplied ID out of bounds";
		return nullptr;
	}

	return mLoadedTextures[aID.GetID()];
}

#include <D3Dcommon.h>
Texture* ResourceManager::LoadTexture(RawTextureData aData)
{
	// Convert string texture filepath to wstring
	std::wstring wString = std::wstring(aData.filepath.begin(), aData.filepath.end());
	const WCHAR* resultString = wString.c_str();
	HRESULT result;

	auto tpTextureClass = new Texture();
	assert(tpTextureClass);
	// if the single texture could not initialize

	// Create shader resource
	result = D3DX11CreateShaderResourceViewFromFile(mpDevice, resultString, NULL, NULL, &tpTextureClass->srv, NULL);
	
	if (FAILED(result))
	{
		LOG(WARNING) << "ResourceManager: Texture could not initialize " << aData.filepath;
	}


	return tpTextureClass;
}

Texture* ResourceManager::LoadTextureCube(RawTextureData aData)
{
	// Convert string texture filepath to wstring
	std::wstring wString = std::wstring(aData.filepath.begin(), aData.filepath.end());
	const WCHAR* resultString = wString.c_str();
	HRESULT result;

	auto tpTextureClass = new Texture();
	assert(tpTextureClass);
	// if the single texture could not initialize

	
	D3DX11_IMAGE_LOAD_INFO loadSMInfo;

	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* SMTexture = 0;
	
	result = D3DX11CreateTextureFromFile(mpDevice, resultString, &loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0);
	if (FAILED(result))
	{
		LOG(WARNING) << "ResourceManager: LoadTextureCube failed to create texture";
	}

	D3D11_TEXTURE2D_DESC SMTextureDesc;
	SMTexture->GetDesc(&SMTextureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	result = mpDevice->CreateShaderResourceView(SMTexture, &SMViewDesc, &tpTextureClass->srv);

	if (FAILED(result))
	{
		LOG(WARNING) << "ResourceManager create shader resource view failed in create texture cube";
	}

	return tpTextureClass;
}



Material* ResourceManager::LoadCubeMapTexturesFromMaterial(const RawMeshData& aMeshData)
{
	Material* tpMat = new Material();

	assert(tpMat);


	if (this->stringTextureMap.find(aMeshData.diffuseData.filepath) != stringTextureMap.end())
	{
		tpMat->mpDiffuse = stringTextureMap[aMeshData.diffuseData.filepath];
	}
	else
	{
		tpMat->mpDiffuse = TexID((uint32_t)mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTextureCube(aMeshData.diffuseData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.diffuseData.filepath, tpMat->mpDiffuse));
	}

	if (this->stringTextureMap.find(aMeshData.specularData.filepath) != stringTextureMap.end())
	{
		tpMat->mpSpecular = stringTextureMap[aMeshData.specularData.filepath];
	}
	else
	{
		tpMat->mpSpecular = TexID((uint32_t)mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTextureCube(aMeshData.specularData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.specularData.filepath, tpMat->mpSpecular));
	}

	if (this->stringTextureMap.find(aMeshData.normalData.filepath) != stringTextureMap.end())
	{
		tpMat->mpNormal = stringTextureMap[aMeshData.normalData.filepath];
	}
	else
	{
		tpMat->mpNormal = TexID((uint32_t)mLoadedTextures.size());

		mLoadedTextures.push_back(LoadTextureCube(aMeshData.normalData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.normalData.filepath, tpMat->mpNormal));
	}

	return tpMat;
}

Material* ResourceManager::LoadTexturesFromMaterial(const RawMeshData& aMeshData)
{
	Material* tpMat = new Material();

	assert(tpMat);

	
	if (this->stringTextureMap.find(aMeshData.diffuseData.filepath) != stringTextureMap.end())
	{
		tpMat->mpDiffuse = stringTextureMap[aMeshData.diffuseData.filepath];
	}
	else
	{
		tpMat->mpDiffuse = TexID((uint32_t)mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTexture(aMeshData.diffuseData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.diffuseData.filepath, tpMat->mpDiffuse));
	}

	if (this->stringTextureMap.find(aMeshData.specularData.filepath) != stringTextureMap.end())
	{
		tpMat->mpSpecular = stringTextureMap[aMeshData.specularData.filepath];
	}
	else
	{
		tpMat->mpSpecular = TexID((uint32_t)mLoadedTextures.size());
		mLoadedTextures.push_back(LoadTexture(aMeshData.specularData));
		stringTextureMap.insert(std::pair<std::string, TexID>(aMeshData.specularData.filepath, tpMat->mpSpecular));
	}

	if (this->stringTextureMap.find(aMeshData.normalData.filepath) != stringTextureMap.end())
	{
		tpMat->mpNormal = stringTextureMap[aMeshData.normalData.filepath];
	}
	else
	{
		tpMat->mpNormal = TexID((uint32_t)mLoadedTextures.size());

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
		ModelID id = ModelID((uint32_t)modelData.size());
		tModels.push_back(id);

		modelData.push_back(tMod);
	}

	stringModelMap[aFilePath] = tModels;
	LOG(INFO) << "ModelLoading: Finished loading model " << aFilePath;
	return tModels;
}

