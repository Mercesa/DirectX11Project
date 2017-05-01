#include "d3dclass.h"

#include <iostream>
#include <memory>

#include "d3dDXGIManager.h"
#include "d3dSwapchain.h"
#include "d3dDepthStencil.h"
#include "d3dRasterizerState.h"

#include "easylogging++.h"

D3DClass::D3DClass()
{
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}


bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	HRESULT result;
	int numerator, denominator;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Store the vsync setting.
	m_vsync_enabled = vsync;

	std::unique_ptr<d3dDXGIManager> md3dDXGIManager = std::make_unique<d3dDXGIManager>();
	md3dDXGIManager->Create(screenWidth, screenHeight, numerator, denominator);

	featureLevel = D3D_FEATURE_LEVEL_11_0;
	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, 0, 0, D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext);

	if (FAILED(result))
	{
		LOG(ERROR) << "device failed creation";
		return false;
	}

	mpSwapChain = std::make_unique<d3dSwapchain>(md3dDXGIManager->GetFactory(), this->mpDevice.Get());
	
	bool swapChainCreationResult = mpSwapChain->Create(screenWidth, screenHeight, numerator, screenHeight, m_vsync_enabled, fullscreen, hwnd);

	if (!swapChainCreationResult)
	{
		LOG(ERROR) << "swapchain failed creation";
		return false;
	}
	
	// Get the pointer to the back buffer.
	result = mpSwapChain->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to get back buffer ptr from swapchain";
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = mpDevice->CreateRenderTargetView(backBufferPtr, NULL, &mpRenderTargetView);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to create render target view";
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;
	

	mpDepthStencil = std::make_unique<d3dDepthStencil>(mpDevice.Get(), mpDeviceContext.Get());

	mpDepthStencil->Create(screenWidth, screenHeight);

	// Set the depth stencil state.
	mpDeviceContext->OMSetDepthStencilState(mpDepthStencil->GetDepthStencilState(), 1);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	mpDeviceContext->OMSetRenderTargets(1, mpRenderTargetView.GetAddressOf(), mpDepthStencil->GetDepthStencilView());

	mpRasterizerState = std::make_unique<d3dRasterizerState>(mpDevice.Get(), mpDeviceContext.Get());
	mpRasterizerState->Create();

	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	mpDeviceContext->RSSetState(mpRasterizerState->m_rasterState.Get());

	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	mpDeviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	XMStoreFloat4x4(&mProjectionmatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth));
	// Initialize the world matrix to the identity matrix.


	XMMATRIX tIdentityMat = XMMatrixIdentity();
	XMMATRIX tScaleMat = tScaleMat = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	
	XMStoreFloat4x4(&mWorldMatrix, XMMatrixMultiply(tIdentityMat, tScaleMat));
	
	//Create an orthographic projection matrix for 2D rendering.
	XMStoreFloat4x4(&mOrthoMatrix, XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth));

	return true;
}


void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	mpSwapChain->Shutdown();


	mpRenderTargetView.Reset();
	mpDeviceContext.Reset();
	mpDevice.Reset();

	return;
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	mpDeviceContext->ClearRenderTargetView(mpRenderTargetView.Get(), color);

	// Clear the depth buffer.
	mpDeviceContext->ClearDepthStencilView(mpDepthStencil->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
	mpSwapChain->Swap(m_vsync_enabled);
}


ID3D11Device* D3DClass::GetDevice()
{
	return mpDevice.Get();
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return mpDeviceContext.Get();
}


void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = XMLoadFloat4x4(&mProjectionmatrix);
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = XMLoadFloat4x4(&mOrthoMatrix);
}