#include "d3dDepthStencil.h"

#include <cassert>


d3dDepthStencil::d3dDepthStencil(ID3D11Device* const aDevice, ID3D11DeviceContext* aContext) : mPDevice(aDevice), mPDeviceContext(aContext)
{
}


d3dDepthStencil::~d3dDepthStencil()
{
}


ID3D11Texture2D* const d3dDepthStencil::GetDepthTexture() 
{
	assert(mDepthStencilBufferTexture.Get() != nullptr);
	return mDepthStencilBufferTexture.Get(); 
}


ID3D11DepthStencilState* const d3dDepthStencil::GetDepthStencilState() 
{ 
	assert(mDepthStencilState.Get() != nullptr);
	return mDepthStencilState.Get(); 
}


ID3D11DepthStencilView* const d3dDepthStencil::GetDepthStencilView() 
{ 
	assert(mDepthStencilView.Get() != nullptr);
	return mDepthStencilView.Get(); 
}


bool d3dDepthStencil::Create(int aScreenWidth, int aScreenHeight)
{
	HRESULT result;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = aScreenWidth;
	depthBufferDesc.Height = aScreenHeight;
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
	result = mPDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilBufferTexture);
	if (FAILED(result))
	{
		return false;
	}

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

	// Create the depth stencil state.
	result = mPDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	mPDeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = mPDevice->CreateDepthStencilView(mDepthStencilBufferTexture.Get(), &depthStencilViewDesc, &mDepthStencilView);

	if (FAILED(result))
	{
		return false;
	}

	return true;
}