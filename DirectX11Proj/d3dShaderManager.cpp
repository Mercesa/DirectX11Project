#include "d3dShaderManager.h"

#include <cassert>

#include "easylogging++.h"

#include "ShaderHelperFunctions.h"
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"


d3dShaderManager::d3dShaderManager()
{
}


d3dShaderManager::~d3dShaderManager()
{
}


bool d3dShaderManager::InitializeShaders(ID3D11Device* const apDevice)
{
	// Load vertex and pixel shader
	ShaderInfo tShaderInfo = ShaderInfo("Shaders\\VS_texture.hlsl", "TextureVertexShader", "vs_5_0", EVERTEX);
	mShadersInfo.push_back(tShaderInfo);

	tShaderInfo = ShaderInfo("Shaders\\PS_texture.hlsl", "TexturePixelShader", "ps_5_0", EPIXEL);
	mShadersInfo.push_back(tShaderInfo);



	tShaderInfo = ShaderInfo("Shaders\\VS_color.hlsl", "ColorVertexShader", "vs_5_0", EVERTEX);
	mShadersInfo.push_back(tShaderInfo);

	tShaderInfo = ShaderInfo("Shaders\\PS_color.hlsl", "ColorPixelShader", "ps_5_0", EPIXEL);
	mShadersInfo.push_back(tShaderInfo);

	LOG(INFO) << "ShaderManager: Finished initializing all shaders";

	// Load actual shaders
	if (LoadShaders(apDevice) == false)
	{
		LOG(FATAL) << "Shader loading failed!";
		return false;
	}

	LOG(INFO) << "ShaderManager: Finished loading all shaders";

	return true;
}


d3dShaderVS* const d3dShaderManager::GetVertexShader(const char* aShaderPath)
{
	auto it = mVertexShaders.find(aShaderPath);
	
	if (it != mVertexShaders.end())
	{
		return it->second.get();
	}

	else
	{
		LOG(WARNING) << "Vertex Shader - " << aShaderPath << " - not found in currently loaded shaders";
		assert(true);
		return nullptr;
	}
}


d3dShaderPS* const d3dShaderManager::GetPixelShader(const char* aShaderPath)
{
	auto it = mPixelShaders.find(aShaderPath);
	
	if (it != mPixelShaders.end())
	{
		return it->second.get();
	}

	else
	{
		LOG(WARNING) << "Pixel Shader - " << aShaderPath << " - not found in currently loaded shaders";
		assert(true);
		return nullptr;
	}
}


bool d3dShaderManager::LoadShaders(ID3D11Device* const apDevice)
{
	HRESULT result;

	for (auto& e : mShadersInfo)
	{
		if (e.mShaderType == EVERTEX)
		{
			auto it = mVertexShaders.find(e.mFilePath);

			// If the shader exists, continue
			if (it != mVertexShaders.end())
			{
				continue;
			}

			else
			{
				std::unique_ptr<d3dShaderVS> tpShaderVS = std::make_unique<d3dShaderVS>();

				if (!tpShaderVS->LoadVertexShader(apDevice, e))
				{
					return false;
				}

				this->mVertexShaders[e.mFilePath] = std::move(tpShaderVS);
			}

		}

		else if (e.mShaderType == EPIXEL)
		{
			auto it = mPixelShaders.find(e.mFilePath);

			// If the shader exists, continue
			if (it != mPixelShaders.end())
			{
				continue;
			}

			else
			{
				std::unique_ptr<d3dShaderPS> tpShaderPS = std::make_unique<d3dShaderPS>();

				if (!tpShaderPS->LoadPixelShader(apDevice, e))
				{
					return false;
				}

				this->mPixelShaders[e.mFilePath] = std::move(tpShaderPS);
			}
		}
	}
	return true;
}
