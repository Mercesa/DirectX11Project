
#include "d3dRenderTexture.h"
#include "d3d11HelperFile.h"
#include "easylogging++.h"

d3dRenderTexture::d3dRenderTexture()
{}


d3dRenderTexture::d3dRenderTexture(const d3dRenderTexture& other)
{
}


d3dRenderTexture::~d3dRenderTexture()
{
}


bool d3dRenderTexture::Initialize(ID3D11Device*const device, uint32_t textureWidth, uint32_t textureHeight, float screenDepth, float screenNear)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;


	mpRenderTargetTexture = CreateSimpleRenderTargetTexture(device, textureWidth, textureHeight);
	mpRenderTargetView = CreateSimpleRenderTargetView(device, mpRenderTargetTexture.Get());
	mpShaderResourceView = CreateSimpleShaderResourceView(device, mpRenderTargetTexture.Get());
	mpDepthStencilBuffer = CreateSimpleDepthTexture(device, textureWidth, textureHeight);
	mpDepthStencilView = CreateSimpleDepthTextureShader(device, mpDepthStencilBuffer.Get());
	
	// Setup the viewport for rendering.
    m_viewport.Width = (float)textureWidth;
    m_viewport.Height = (float)textureHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

	return true;
}


bool d3dRenderTexture::InitializeWithBackbuffer(ID3D11Device*const aDevice, IDXGISwapChain* const aSwapChain, uint32_t aTextureWidth, uint32_t aTextureHeight, float aNear, float aDepth)
{
	HRESULT result;

	mpDepthStencilBuffer = CreateSimpleDepthTexture(aDevice, aTextureWidth, aTextureHeight);
	mpDepthStencilView = CreateSimpleDepthstencilView(aDevice, mpDepthStencilBuffer.Get());
	mpRenderTargetView = CreateRenderTargetViewFromSwapchain(aDevice, aSwapChain);

	return true;
}
