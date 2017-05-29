#pragma once

#include <map>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>
#include <windows.h>


class d3dShaderVS;
class d3dShaderPS;

enum eShaderTypes { EVERTEX, EPIXEL };


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

	bool InitializeShaders(ID3D11Device* const apDevice);

	d3dShaderVS* const GetVertexShader(const char* aShaderPath);
	d3dShaderPS* const GetPixelShader(const char* aShaderPath);

private:
	std::vector<ShaderInfo> mShadersInfo;

	std::map<const char*, std::unique_ptr<d3dShaderVS>> mVertexShaders;
	std::map<const char*, std::unique_ptr<d3dShaderPS>>mPixelShaders;

	bool LoadShaders(ID3D11Device* const apDevice);
};

