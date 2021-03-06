#pragma once

#include <map>
#include <vector>
#include <memory>

#include <d3d11_1.h>
#include <wrl.h>
#include <windows.h>

#include "d3d11HelperFile.h"


enum eShaderTypes { EVERTEX, EPIXEL, ECOMPUTE };


struct ShaderInfo
{
	ShaderInfo(const char* aFilePath, const char* aEntryPoint, const char* aShaderProfile, eShaderTypes aShaderType) : mFilePath(aFilePath), mEntryPoint(aEntryPoint), mShaderProfile(aShaderProfile), mShaderType(aShaderType)
	{

	}

	const char* mFilePath;
	const char* mEntryPoint;
	const char* mShaderProfile;
	eShaderTypes mShaderType;
};

class d3dShaderManager
{
public:
	// I am aware there are other shader types, but if im not using them there's no use in adding them to the enum


	d3dShaderManager();
	~d3dShaderManager();

	void ReleaseResources();

	bool InitializeShaders(ID3D11Device* const apDevice);

	VertexShader* const GetVertexShader(const char* aShaderPath);
	PixelShader* const GetPixelShader(const char* aShaderPath);
	ComputeShader* const GetComputeShader(const char* aShaderPath);

private:
	std::vector<ShaderInfo> mShadersInfo;

	std::map<const char*, std::unique_ptr<VertexShader>> mVertexShaders;
	std::map<const char*, std::unique_ptr<PixelShader>>	mPixelShaders;
	std::map<const char*, std::unique_ptr<ComputeShader>> mComputeShaders;

	bool LoadShaders(ID3D11Device* const apDevice);
};

