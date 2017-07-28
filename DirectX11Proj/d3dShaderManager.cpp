#include "d3dShaderManager.h"

#include <cassert>

#include "easylogging++.h"

#include "ShaderHelperFunctions.h"


d3dShaderManager::d3dShaderManager(){}

d3dShaderManager::~d3dShaderManager(){}


void d3dShaderManager::ReleaseResources()
{
	for (auto& e : mVertexShaders)
	{
		ReleaseVertexShader(e.second.get());
	}

	for (auto& e : mPixelShaders)
	{
		ReleasePixelShader(e.second.get());
	}
}

bool d3dShaderManager::InitializeShaders(ID3D11Device* const apDevice)
{
	// Load vertex and pixel shader
	mShadersInfo.push_back(ShaderInfo("Shaders\\VS_texture.hlsl", "TextureVertexShader", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\PS_texture.hlsl", "TexturePixelShader", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\VS_color.hlsl", "ColorVertexShader", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\PS_color.hlsl", "ColorPixelShader", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\VS_shadow.hlsl", "ShadowVertexShader", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\PS_shadow.hlsl", "ShadowPixelShader", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\VS_depth.hlsl", "DepthVertexShader", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\PS_depth.hlsl", "DepthPixelShader", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\fullScreenQuad_VS.hlsl", "VertShader", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\fullScreenQuad_PS.hlsl", "PixShader", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\VS_GBufferFill.hlsl", "GBufferFillVertexShader", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\PS_GBufferFill.hlsl", "GbufferFillPixelShader", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\VS_DeferredLighting.hlsl", "VSDeferredLighting", "vs_5_0", EVERTEX));
	mShadersInfo.push_back(ShaderInfo("Shaders\\PS_DeferredLighting.hlsl", "PSDeferredLighting", "ps_5_0", EPIXEL));

	mShadersInfo.push_back(ShaderInfo("Shaders\\CS_GuassianBlur.hlsl", "CSMain", "cs_5_0", ECOMPUTE));

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


VertexShader* const d3dShaderManager::GetVertexShader(const char* aShaderPath)
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


PixelShader* const d3dShaderManager::GetPixelShader(const char* aShaderPath)
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

inline wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

#include "ShaderHelperFunctions.h"


bool LoadVertexShader(ID3D11Device* const apDevice, ShaderInfo& aInfo, VertexShader* const aVertexShader)
{

	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	// Fill array with this function
	CreateVertexbufferLayoutDefault(polygonLayout);

	HRESULT result;


	wchar_t* tWstring = convertCharArrayToLPCWSTR(aInfo.mFilePath);

	ID3D10Blob* tBlobRef = nullptr;
	LoadShaderWithErrorChecking(tWstring, (LPCSTR)aInfo.mEntryPoint, (LPCSTR)aInfo.mShaderProfile, tBlobRef);

	result = apDevice->CreateVertexShader(tBlobRef->GetBufferPointer(), tBlobRef->GetBufferSize(), NULL, &aVertexShader->shader);

	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " failed to load";
		return false;
	}


	// Get a count of the elements in the layout.
	uint32_t tNumElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = apDevice->CreateInputLayout(polygonLayout, tNumElements, tBlobRef->GetBufferPointer(),
		tBlobRef->GetBufferSize(), &aVertexShader->inputLayout);

	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " input layout failed to create";
		return false;
	}


	delete[] tWstring;

	return true;

}

bool LoadPixelShader(ID3D11Device* const apDevice, ShaderInfo& aInfo, PixelShader* const aPixelShader)
{
	HRESULT result;

	wchar_t* tWstring = convertCharArrayToLPCWSTR(aInfo.mFilePath);

	ID3D10Blob* tBlobRef = nullptr;
	LoadShaderWithErrorChecking(tWstring, (LPCSTR)aInfo.mEntryPoint, (LPCSTR)aInfo.mShaderProfile, tBlobRef);

	result = apDevice->CreatePixelShader(tBlobRef->GetBufferPointer(), tBlobRef->GetBufferSize(), NULL, &aPixelShader->shader);

	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " failed to load";
		return false;
	}

	delete[] tWstring;

	return true;
}

bool LoadPixelShader(ID3D11Device* const apDevice, ShaderInfo& aInfo, ComputeShader* const aPixelShader)
{
	HRESULT result;

	wchar_t* tWstring = convertCharArrayToLPCWSTR(aInfo.mFilePath);

	ID3D10Blob* tBlobRef = nullptr;
	LoadShaderWithErrorChecking(tWstring, (LPCSTR)aInfo.mEntryPoint, (LPCSTR)aInfo.mShaderProfile, tBlobRef);

	result = apDevice->CreateComputeShader(tBlobRef->GetBufferPointer(), tBlobRef->GetBufferSize(), NULL, &aPixelShader->shader);

	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " failed to load";
		return false;
	}

	delete[] tWstring;

	return true;
}


bool d3dShaderManager::LoadShaders(ID3D11Device* const apDevice)
{
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
				std::unique_ptr<VertexShader> tpShaderVS = std::make_unique<VertexShader>();

				if (!LoadVertexShader(apDevice, e, tpShaderVS.get()))
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
				std::unique_ptr<PixelShader> tpShaderPS = std::make_unique<PixelShader>();

				if (!LoadPixelShader(apDevice, e, tpShaderPS.get()))
				{
					return false;
				}

				this->mPixelShaders[e.mFilePath] = std::move(tpShaderPS);
			}
		}


		else if (e.mShaderType == ECOMPUTE)
		{
			auto it = mPixelShaders.find(e.mFilePath);

			// If the shader exists, continue
			if (it != mPixelShaders.end())
			{
				continue;
			}

			else
			{
				std::unique_ptr<ComputeShader> tpShaderCS = std::make_unique<ComputeShader>();

				if (!LoadPixelShader(apDevice, e, tpShaderCS.get()))
				{
					return false;
				}

				this->mComputeShaders[e.mFilePath] = std::move(tpShaderCS);
			}
		}
	}
	return true;
}
