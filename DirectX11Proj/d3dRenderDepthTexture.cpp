#include "d3dRenderDepthTexture.h"

#include "easylogging++.h"

#include "FormatConversionHelper.h"

d3dRenderDepthTexture::d3dRenderDepthTexture()
{
}


d3dRenderDepthTexture::~d3dRenderDepthTexture()
{
}

bool d3dRenderDepthTexture::Initialize(ID3D11Device* const aDevice, uint32_t aTextureWidth, uint32_t aTextureHeight, float aScreenNear, float aScreenFar)
{
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	HRESULT result;

	DXGI_FORMAT dsvFormat = GetDepthResourceFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	DXGI_FORMAT srvFormat = GetDepthSRVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);

	// Create depth buffer
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));

	depthTextureDesc.Width = aTextureWidth;
	depthTextureDesc.Height = aTextureHeight;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = dsvFormat;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	result = aDevice->CreateTexture2D(&depthTextureDesc, NULL, &mpDepthTexture);
	if (FAILED(result))
	{
		LOG(INFO) << "Failed to create render depth texture";
		return false;
	}
	
	// Create depth stencil view
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = dsvFormat;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = aDevice->CreateDepthStencilView(mpDepthTexture.Get(), &depthStencilViewDesc, mpDepthStenciLView.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Create shader resource view
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = srvFormat;;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = aDevice->CreateShaderResourceView(mpDepthTexture.Get(), &shaderResourceViewDesc, mpShaderResourceView.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	return true;
}