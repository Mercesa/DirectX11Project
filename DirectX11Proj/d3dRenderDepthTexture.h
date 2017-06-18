#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <windowsx.h>
#include <wrl.h>

#include <cstdint>

class d3dRenderDepthTexture
{
public:
	d3dRenderDepthTexture();
	~d3dRenderDepthTexture();

	bool Initialize(ID3D11Device* const aDevice, uint32_t aTextureWidth, uint32_t aTextureHieght, float aScreenNear, float aScreenFar);


	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mpDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mpDepthTexture;
	
};

