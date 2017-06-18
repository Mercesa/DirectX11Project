#pragma once



#include <windowsx.h>
#include <wrl.h>

#include <fcntl.h>
#include <d3d11.h>
#include "easylogging++.h"


struct Texture
{
	ID3D11Texture2D* texture;
	ID3D11DepthStencilView* dsv;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
};


/*
***********************
* RASTERIZER FUNCTIONS
***********************
*/
static ID3D11RasterizerState* CreateRSDefault(ID3D11Device* const aDevice)
{
	ID3D11RasterizerState* tRaster = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;


	result = aDevice->CreateRasterizerState(&rasterDesc, &tRaster);
	
	if (FAILED(result))
	{
		LOG(INFO) << "CreateDefaultRasterizerState failed";
		return nullptr;
	}

	return tRaster;
}


static ID3D11RasterizerState* CreateRSWireFrame(ID3D11Device* const aDevice)
{
	ID3D11RasterizerState* tRaster = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;


	result = aDevice->CreateRasterizerState(&rasterDesc, &tRaster);

	if (FAILED(result))
	{
		LOG(ERROR) << "CreateDefaultRasterizerState failed";
		return nullptr;
	}

	return tRaster;
}


/*
***********************************
**SAMPLER STATE DEFAULT FUNCTIONS**
***********************************
*/

static ID3D11SamplerState* CreateSamplerAnisotropicWrap(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerAnisotropicWrap failed";
		return false;
	}

	return sampler;
}

static ID3D11SamplerState* CreateSamplerPointClamp(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerPointClamp failed";
		return false;
	}

	return sampler;
}

static ID3D11SamplerState* CreateSamplerLinearClamp(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.	
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerPointClamp failed";
		return false;
	}

	return sampler;
}


/*
***************************
**DEPTH STENCIL STATE FUNCTIONS**
***************************
*/
static ID3D11DepthStencilState* CreateDepthStateDefault(ID3D11Device* const aDevice)
{
	ID3D11DepthStencilState* tDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	HRESULT result;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = aDevice->CreateDepthStencilState(&depthStencilDesc, &tDepthStencilState);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth-stencil state";
		return nullptr;
	}

	return tDepthStencilState;
}


/*
***************************
**DEPTH STENCIL VIEW FUNCTIONS**
***************************
*/
static ID3D11DepthStencilView* CreateSimpleDepthTextureShader(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture)
{

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ID3D11DepthStencilView* tDsv = nullptr;
	HRESULT result;
	// Initialize the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = aDevice->CreateDepthStencilView(aTexture, &depthStencilViewDesc, &tDsv);
	if (FAILED(result))
	{
		return false;
	}
}


/*
***********************************
**DEPTH STENCIL TEXTURE FUNCTIONS**
***********************************
*/

static ID3D11Texture2D* CreateSimpleDepthTextureVisibleShader(ID3D11Device* const aDevice, uint32_t aWidth, uint32_t aHeight)
{
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	ID3D11Texture2D* tTexture = nullptr;
	HRESULT result;

	// Create depth buffer
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));

	depthTextureDesc.Width = aWidth;
	depthTextureDesc.Height = aHeight;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	result = aDevice->CreateTexture2D(&depthTextureDesc, NULL, &tTexture);

	if (FAILED(result))
	{
		LOG(ERROR) << "CreateTexture2DDSVDefault failed";
		return nullptr;
	}

	return tTexture;
}



static ID3D11ShaderResourceView* CreateSimpleShaderResourceViewDepth(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture)
{
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ID3D11ShaderResourceView* srv = nullptr;

	// Create shader resource view
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = -1;

	HRESULT result = aDevice->CreateShaderResourceView(aTexture, &shaderResourceViewDesc, &srv);
	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create shader resource view in d3dRenderDepthTexture";
		return nullptr;
	}

	return srv;
}

/*
********************************
**RENDER TARGET VIEW FUNCTIONS**
********************************
*/
static ID3D11Texture2D* CreateSimpleRenderTargetTexture(ID3D11Device* const aDevice, uint32_t aWidth, uint32_t aHeight)
{
	HRESULT result;
	D3D11_TEXTURE2D_DESC textureDesc;
	ID3D11Texture2D* tTexture = nullptr;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = aWidth;
	textureDesc.Height = aHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = aDevice->CreateTexture2D(&textureDesc, NULL, &tTexture);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateTexture2DRTVDefault failed";
		return nullptr;
	}

	return tTexture;
}

static ID3D11Texture2D* CreateSimpleDepthTexture(ID3D11Device* const aDevice, uint32_t aWidth, uint32_t aHeight)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	HRESULT result;
	ID3D11Texture2D* texture;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = aWidth;
	depthBufferDesc.Height = aWidth;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = aDevice->CreateTexture2D(&depthBufferDesc, NULL, &texture);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSimpleDepthBuffer failed";
		return false;
	}

	return texture;
}

static ID3D11RenderTargetView* CreateSimpleRenderTargetView(ID3D11Device* const aDevice, ID3D11Texture2D* aTexture)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ID3D11RenderTargetView* rtv = nullptr;
	HRESULT result;

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = aDevice->CreateRenderTargetView(aTexture, &renderTargetViewDesc, &rtv);
	if (FAILED(result))
	{
		return false;
		return nullptr;
	}

	return rtv;
}


static ID3D11DepthStencilView* CreateSimpleDepthstencilView(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ID3D11DepthStencilView* dsv;
	HRESULT result;

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = aDevice->CreateDepthStencilView(aTexture, &depthStencilViewDesc, &dsv);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil view";
		return nullptr;
	}

	return dsv;
}

static ID3D11ShaderResourceView* CreateSimpleShaderResourceView(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ID3D11ShaderResourceView* srv;
	HRESULT result;
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = aDevice->CreateShaderResourceView(aTexture, &shaderResourceViewDesc, &srv);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSimpleShaderResourceView failed";
		return nullptr;
	}

	return srv;
}


static ID3D11RenderTargetView* CreateRenderTargetViewFromSwapchain(ID3D11Device* const aDevice, IDXGISwapChain* const aSwapChain)
{
	ID3D11Texture2D* backBufferPtr;
	ID3D11RenderTargetView* rtv;

	HRESULT result = aSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to get back buffer ptr from swapchain";
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = aDevice->CreateRenderTargetView(backBufferPtr, NULL, &rtv);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to create render target view";
		return false;
	}

	// Release the backbuffer ptr
	backBufferPtr->Release();
	backBufferPtr = 0;

	return rtv;
}

//static ID3D11RenderTargetView* CreateTexture2DRTVDefault(ID3D11Device* const aDevice, uint32_t aWidth, uint32_t aHeight)
//{
//	HRESULT result;
//	D3D11_TEXTURE2D_DESC textureDesc;
//	ID3D11Texture2D* tTexture = nullptr;
//
//	// Initialize the render target texture description.
//	ZeroMemory(&textureDesc, sizeof(textureDesc));
//
//	// Setup the render target texture description.
//	textureDesc.Width = aWidth;
//	textureDesc.Height = aHeight;
//	textureDesc.MipLevels = 1;
//	textureDesc.ArraySize = 1;
//	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
//	textureDesc.SampleDesc.Count = 1;
//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
//	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//	textureDesc.CPUAccessFlags = 0;
//	textureDesc.MiscFlags = 0;
//
//	// Create the render target texture.
//	result = aDevice->CreateTexture2D(&textureDesc, NULL, &tTexture);
//	if (FAILED(result))
//	{
//		LOG(ERROR) << "CreateTexture2DRTVDefault failed";
//		return nullptr;
//	}
//
//	return tTexture;
//}

