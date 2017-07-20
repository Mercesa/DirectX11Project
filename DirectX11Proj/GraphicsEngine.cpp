#include "GraphicsEngine.h"

#include "easylogging++.h"

#include "d3dShaderManager.h"
#include "d3dDXGIManager.h"
#include "d3dSwapchain.h"


GraphicsEngine::GraphicsEngine() : hasBeenInitialized(false)
{
}


GraphicsEngine::~GraphicsEngine()
{
}


bool GraphicsEngine::Initialize(int aScreenWidth, int aScreenHeight, HWND hwnd)
{
	if (hasBeenInitialized)
	{
		LOG(FATAL) << "Attempt to initialize the engine twice";
		return false;
	}
	int numerator, denominator;

	if (!CreateDevice())
	{
		return false;
	}

	mpDXGIManager = std::make_unique<d3dDXGIManager>();
	
	if (!mpDXGIManager->Create(aScreenWidth, aScreenHeight, numerator, denominator))
	{
		LOG(FATAL) << "Failed to create DXGI manager";
		return false;
	}

	mpSwapChain = std::make_unique<d3dSwapchain>();
	bool swapChainCreationResult = mpSwapChain->Create(aScreenWidth, aScreenHeight, numerator, denominator, true, false, hwnd);

	if (!swapChainCreationResult)
	{
		LOG(FATAL) << "swapchain failed creation";
		return false;
	}
	
	if (!InitializeSamplers())
	{
		return false;
	}

	mpShaderManager = std::make_unique<d3dShaderManager>();
	if (!mpShaderManager->InitializeShaders(mpDevice.Get()))
	{
		return false;
	}
	
	// Graphics engine correctly has been initialized
	hasBeenInitialized = true;

	return true;
}


bool GraphicsEngine::InitializeSamplers()
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	D3D11_SAMPLER_DESC samplerDesc;

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
	result = this->mpDevice->CreateSamplerState(&samplerDesc, &mpAnisotropicWrapSampler);
	if (FAILED(result))
	{
		LOG(WARNING) << "Failed to sampler states";
		return false;
	}

	return true;
}


bool GraphicsEngine::CreateDevice()
{
	assert(mpDevice.Get() == nullptr);

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
		LOG(ERROR) << "Device Creation failed";
		return false;
	}

	return true;
}

void GraphicsEngine::EndScene()
{
	mpSwapChain->Swap(true);
}


ID3D11Device* const GraphicsEngine::GetDevice()
{
	assert(mpDevice.Get() != nullptr);
	return mpDevice.Get();
}


ID3D11DeviceContext* const GraphicsEngine::GetDeviceContext()
{
	assert(mpDeviceContext.Get() != nullptr);
	return mpDeviceContext.Get();
}


ID3D11SamplerState* const GraphicsEngine::GetAnisotropicWrapSampler()
{
	assert(this->mpAnisotropicWrapSampler.Get() != nullptr);
	return mpAnisotropicWrapSampler.Get();
}

void GraphicsEngine::SetProjectionMatrix(XMFLOAT4X4 aProjMatrix)
{
	mProjectionMatrix = aProjMatrix;
}

void GraphicsEngine::SetViewMatrix(XMFLOAT4X4 aViewMatrix)
{
	mViewMatrix = aViewMatrix;
}

XMFLOAT4X4 GraphicsEngine::GetProjectionMatrix()
{
	return mProjectionMatrix;
}

XMFLOAT4X4 GraphicsEngine::GetViewMatrix()
{
	return mViewMatrix;
}

d3dShaderManager* const GraphicsEngine::GetShaderManager()
{
	assert(this->mpShaderManager.get() != nullptr);
	return mpShaderManager.get();
}

d3dDXGIManager* const GraphicsEngine::GetDXGIManager()
{
	assert(this->mpDXGIManager.get() != nullptr);
	return mpDXGIManager.get();
}

d3dSwapchain* const GraphicsEngine::GetSwapchain()
{
	assert(this->mpSwapChain.get() != nullptr);
	return mpSwapChain.get();
}