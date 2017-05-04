#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>

struct ShaderInfo;

class d3dShaderPS
{
public:
	d3dShaderPS();
	~d3dShaderPS();

	bool LoadPixelShader(ID3D11Device* const apDevice, ShaderInfo& aInfo);
	
	ID3D11PixelShader* const GetPixelShader() { return mpPixelShader.Get(); }


private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mpPixelShader;
	Microsoft::WRL::ComPtr<ID3D10Blob> mpDataBlob;
};

