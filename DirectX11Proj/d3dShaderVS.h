#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>

struct ShaderInfo;

class d3dShaderVS
{
public:
	d3dShaderVS();
	~d3dShaderVS();

	bool LoadVertexShader(ID3D11Device* const apDevice, ShaderInfo& aInfo);

	ID3D11VertexShader* const GetVertexShader() { return mpVertexShader.Get(); }
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mpLayout;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mpVertexShader;
	Microsoft::WRL::ComPtr<ID3D10Blob> mpDataBlob;
};

