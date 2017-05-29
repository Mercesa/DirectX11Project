#include "d3dGraphicsEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "easylogging++.h"

d3dGraphicsEngine::d3dGraphicsEngine() 
{
	mpShaderManager = std::make_unique<d3dShaderManager>();
}


d3dGraphicsEngine::~d3dGraphicsEngine()
{
}

void d3dGraphicsEngine::StartStandardTargets(float clearColor[4], D3D11_VIEWPORT aViewPort )
{
	mpDeviceContext->RSSetViewports(1, &aViewPort);
	mpDeviceContext->RSSetState(mRasterState.Get());
	mpDeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), 1);

	mpDeviceContext->ClearRenderTargetView(mMainRTV.Get(), clearColor);
	mpDeviceContext->ClearDepthStencilView(mMainDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* tRTV = this->mMainRTV.Get();

	mpDeviceContext->OMSetRenderTargets(1, &tRTV, mMainDSV.Get());
}

bool d3dGraphicsEngine::Initialize()
{
	if (!CreateDevice())
	{
		return false;
	}
	LOG(INFO) << "Device and context created";

	if (!CreateDXGI())
	{
		return false;
	}
	LOG(INFO) << "Created DXGI";

	if (!CreateSwapchain())
	{
		return false;
	}
	LOG(INFO) << "Created swapchain";

	if (!CreateDepthStencil())
	{
		return false;
	}
	LOG(INFO) << "Created depth stencil state, view and texture";

	if (!CreateRasterState())
	{
		return false;
	}
	LOG(INFO) << "Created raster state";

	return true;
}


bool d3dGraphicsEngine::CreateRasterState()
{
	HRESULT result;
	D3D11_RASTERIZER_DESC rasterDesc;

	// Setup the raster description which will determine how and what polygons will be drawn.
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

	result = mpDevice->CreateRasterizerState(&rasterDesc, &mRasterState);
	if (FAILED(result))
	{
		LOG(INFO) << "Rasterizer state failed to initialize";
		return false;
	}
}

bool d3dGraphicsEngine::SetWindowHandle(HWND aHWND)
{
	this->hwnd = aHWND;
	return true;
}

// Resize all resources or something like that.
bool d3dGraphicsEngine::OnResize()
{
	return true;
}

bool d3dGraphicsEngine::MatchDisplayMode(int aWidth, int aHeight, int& aNumerator, int& aDenominator, int aNumModes, DXGI_MODE_DESC* aModeArray)
{
	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (int i = 0; i < aNumModes; i++)
	{
		if (aModeArray[i].Width == (unsigned int)800)
		{
			if (aModeArray[i].Height == (unsigned int)600)
			{
				aNumerator = aModeArray[i].RefreshRate.Numerator;
				aDenominator = aModeArray[i].RefreshRate.Denominator;
			}
		}
	}

	return true;
}


bool d3dGraphicsEngine::CreateDevice()
{
	HRESULT result;
	D3D_FEATURE_LEVEL featureLevel;
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	featureLevel = D3D_FEATURE_LEVEL_11_0;

	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, 0, 0, D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext);

	
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed creating Device and/or context";
		return false;
	}






	return true;
}


bool d3dGraphicsEngine::CreateDXGIFactoryObject()
{
	HRESULT result;
	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&mFactory);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to create DXGIFactory";
		return false;
	}
}


bool d3dGraphicsEngine::EnumAdapters()
{
	// Use the factory to create an adapter for the primary graphics interface (video card).
	HRESULT result;
	result = mFactory->EnumAdapters(0, &mAdapter);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to enumerate adapters";
		return false;
	}
	return true;
}

bool d3dGraphicsEngine::CreateDepthStencil()
{
	HRESULT result;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = 800;
	depthBufferDesc.Height = 600;
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
	result = mpDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilBufferTexture);
	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create texture 2D";
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
	result = mpDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil state";
	}

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = mpDevice->CreateDepthStencilView(mDepthStencilBufferTexture.Get(), &depthStencilViewDesc, &mMainDSV);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil view";
		return false;
	}

	return true;

}

bool d3dGraphicsEngine::CreateDXGI()
{
	HRESULT result;
	unsigned int numModes, i;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;


	CreateDXGIFactoryObject();
	EnumAdapters();

	// Enumerate the primary adapter output (monitor).
	result = mAdapter->EnumOutputs(0, &mAdapterOutput);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to enumerate primary adapter output";
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = mAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to get displaymode list";
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = mAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)800)
		{
			if (displayModeList[i].Height == (unsigned int)600)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = mAdapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	mVideoCardMemoryAmount = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);


	stringLength = 0;
	size_t lValue = (size_t)stringLength;

	// Convert the name of the video card to a character array and store it.
	//error = wcstombs_s(&lValue, mVideoCardDescription, 128, adapterDesc.Description, 128);
	//if (error != 0)
	//{
	//	return false;
	//}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;
}


bool d3dGraphicsEngine::CreateSwapchain()
{
	HRESULT result;
	ID3D11Texture2D* backBufferPtr;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = 800;
	swapChainDesc.BufferDesc.Height = 600;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (true)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (false)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	IDXGISwapChain* tSC = mpSwapchain.Get();
	mFactory->CreateSwapChain(mpDevice.Get(), &swapChainDesc, &mpSwapchain);


	result = mpSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to get back buffer ptr from swapchain";
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = mpDevice->CreateRenderTargetView(backBufferPtr, NULL, &mMainRTV);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to create render target view";
		return false;
	}
	return true;
}


void d3dGraphicsEngine::GetVideoCardInfo(char* cardName, int& memory)
{
	//strcpy_s(cardName, 128, mVideoCardDescription);
	//memory = mVideoCardMemoryAmount;
}

bool d3dGraphicsEngine::PresentFrame()
{
	mpSwapchain->Present((true ? 1 : 0), 0);
	return true;
}

bool d3dGraphicsEngine::ShutDown()
{
	HRESULT result;
	Microsoft::WRL::ComPtr<ID3D11Debug> debugDevice;



	//mpShaderManager.release();
	mpDeviceContext.Reset();
	mpSwapchain.Reset();
	mAdapter.Reset();
	mAdapterOutput.Reset();
	mMainRTV.Reset();
	mMainDSV.Reset();

	mDepthStencilBufferTexture.Reset();
	mDepthStencilState.Reset();
	mRasterState.Reset();

	mFactory.Reset();

	result = mpDevice.As(&debugDevice);
	debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to query debug device";
		return false;
	}

	mpDevice.Reset();

	return true;
}


ID3D11Device* const d3dGraphicsEngine::GetDevice()
{ 
	return mpDevice.Get(); 
}


ID3D11DeviceContext* const d3dGraphicsEngine::GetDeviceContext()
{ 
	return mpDeviceContext.Get(); 
}


IDXGISwapChain* const d3dGraphicsEngine::getSwapChain()
{
	assert(mpSwapchain != nullptr);
	return mpSwapchain.Get();
}


bool d3dGraphicsEngine::InitializeShaders()
{
	//return mpShaderManager->InitializeShaders(mpDevice.Get());
	return true;
}


d3dShaderVS* const d3dGraphicsEngine::GetVertexShader(const char* aShaderPath)
{
	return mpShaderManager->GetVertexShader(aShaderPath);
}


d3dShaderPS* const d3dGraphicsEngine::GetPixelShader(const char* aShaderPath)
{
	return mpShaderManager->GetPixelShader(aShaderPath);
}