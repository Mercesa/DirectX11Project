#pragma once

#include <d3d11.h>
#include <D3DX11tex.h>
#include <wrl.h>

class d3dTexture
{
public:
	d3dTexture();
	~d3dTexture();

	bool Initialize(ID3D11Device* const aDevice, const WCHAR* aFilepath);
	void Shutdown();

	ID3D11ShaderResourceView* const GetTexture();

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpTexture;
};

