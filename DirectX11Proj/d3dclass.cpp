#include "d3dclass.h"

#include <iostream>
#include <memory>

#include "d3dDXGIManager.h"
#include "d3dSwapchain.h"
#include "d3dDepthStencil.h"
#include "d3dRasterizerState.h"
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
	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, 0, 0, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_deviceContext);

	if (FAILED(result))
	{
		std::cout << "device failed creation" << std::endl;
		return false;
	}

	mSwapChain = std::make_unique<d3dSwapchain>(md3dDXGIManager->GetFactory(), this->m_device.Get());
	
	bool swapChainCreationResult = mSwapChain->Create(screenWidth, screenHeight, numerator, screenHeight, m_vsync_enabled, fullscreen, hwnd);

	if (!swapChainCreationResult)
	{
		std::cout << "swapchain failed creation" << std::endl;
		return false;
	}
	
	// Get the pointer to the back buffer.
	result = mSwapChain->GetSwapChainPtr()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;
	

	mDepthStencil = std::make_unique<d3dDepthStencil>(m_device.Get(), m_deviceContext.Get());

	mDepthStencil->Create(screenWidth, screenHeight);

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(mDepthStencil->GetDepthStencilState(), 1);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), mDepthStencil->GetDepthStencilView());

	mRasterizerState = std::make_unique<d3dRasterizerState>(m_device.Get(), m_deviceContext.Get());
	mRasterizerState->Create();

	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(mRasterizerState->m_rasterState.Get());

	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// Initialize the world matrix to the identity matrix.
	m_worldMatrix = XMMatrixIdentity();

	// Create an orthographic projection matrix for 2D rendering.
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}


void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	mSwapChain->Shutdown();


	m_renderTargetView.Reset();
	m_deviceContext.Reset();
	m_device.Reset();

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
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(mDepthStencil->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
	mSwapChain->Swap(m_vsync_enabled);
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device.Get();
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext.Get();
}


void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}