#include "Renderer.h"

#include "easylogging++.h"

#include "IScene.h"
#include "d3dConstantBuffer.h"
#include "d3dShaderManager.h"
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"
#include "ResourceManager.h"
#include "d3dMaterial.h"
#include "LightStruct.h"
#include "ConstantBuffers.h"
#include "IObject.h"
#include "GraphicsSettings.h"
using namespace DirectX;

Renderer::Renderer()
{
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(HWND aHwnd)
{
	windowHandle = aHwnd;
	InitializeDirectX();

	CreateConstantBuffers();
	ResourceManager::GetInstance().mpDevice = mpDevice.Get();

	mpShaderManager = std::make_unique<d3dShaderManager>();
	mpShaderManager->InitializeShaders(mpDevice.Get());

}

void Renderer::CreateConstantBuffers()
{
	// Create constant buffers
	mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(MatrixBufferType), nullptr, mpDevice.Get());
	mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(MaterialBufferType), nullptr, mpDevice.Get());
	mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(LightBufferType), nullptr, mpDevice.Get());
	LOG(INFO) << "Constant buffers created";
}

void Renderer::UpdateFrameConstantBuffers(IScene* const aScene)
{
	// Update light constant buffer

	static LightBufferType* dataPtr = new LightBufferType();
	uint32_t bufferNumber;

	// Get a pointer to the data in the constant buffer.

	dataPtr->amountOfLights = static_cast<int>(aScene->mLights.size());

	for (int i = 0; i < aScene->mLights.size(); ++i)
	{
		dataPtr->arr[i].position = aScene->mLights[i]->position;
		dataPtr->arr[i].diffuseColor = aScene->mLights[i]->diffuseColor;
	}

	dataPtr->directionalLight.diffuseColor = aScene->mDirectionalLight->diffuseColor;
	dataPtr->directionalLight.specularColor = aScene->mDirectionalLight->specularColor;
	dataPtr->directionalLight.position = aScene->mDirectionalLight->position;


	mpLightCB->UpdateBuffer((void*)dataPtr, mpDeviceContext.Get());


	bufferNumber = 2;
	ID3D11Buffer* tBuff = mpLightCB->GetBuffer();

	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
}

void Renderer::UpdateObjectConstantBuffers(IObject* const aObject, IScene* const aScene)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	static MaterialBufferType* dataPtr = new MaterialBufferType();
	uint32_t bufferNumber;

	// Update material
	d3dMaterial* const tMat = aObject->mpModel->mMaterial.get();

	dataPtr->hasDiffuse = (int)tMat->mpDiffuse->exists;
	dataPtr->hasSpecular = (int)tMat->mpSpecular->exists;
	dataPtr->hasNormal = (int)tMat->mpNormal->exists;


	mpMaterialCB->UpdateBuffer((void*)dataPtr, mpDeviceContext.Get());
	bufferNumber = 1;
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
	// End update material


	// Update matrix cb

	// very bad code but will fix later either way
	MatrixBufferType* dataPtr2 = new MatrixBufferType();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	worldMatrix = XMLoadFloat4x4(&aObject->mWorldMatrix);
	projectionMatrix = XMLoadFloat4x4(&gProjectionMatrix);
	aScene->GetCamera()->GetViewMatrix(viewMatrix);


	// Transpose the matrices to prepare them for the shader.
	XMMATRIX worldMatrix2 = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrix2 = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrix2 = XMMatrixTranspose(projectionMatrix);

	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	dataPtr2->world = worldMatrix2;
	dataPtr2->view = viewMatrix2;
	dataPtr2->projection = projectionMatrix2;
	dataPtr2->gEyePosX = aScene->GetCamera()->GetPosition().x;
	dataPtr2->gEyePosY = aScene->GetCamera()->GetPosition().y;
	dataPtr2->gEyePosZ = aScene->GetCamera()->GetPosition().z;

	
	mpMatrixCB->UpdateBuffer((void*)dataPtr2, mpDeviceContext.Get());

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	tBuff = mpMatrixCB->GetBuffer();
	// finally set the constant buffer in the vertex shader with the updated values.
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);


	delete dataPtr2;
	// end update matrix cb
}

void Renderer::RenderScene(IScene* const aScene)
{
	float color[4]{ 0.6f, 0.6f, 0.6f, 1.0f };

	mpDeviceContext->RSSetViewports(1, &gViewPort);
	mpDeviceContext->RSSetState(mRasterState.Get());

	mpDeviceContext->ClearRenderTargetView(mpRenderTargetView.Get(), color);
	mpDeviceContext->ClearDepthStencilView(mpDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState.Get(), 1);
	mpDeviceContext->OMSetRenderTargets(1, mpRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

	aScene->GetCamera()->UpdateViewMatrix();

	d3dShaderVS*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_texture.hlsl");
	d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_texture.hlsl");

	UpdateFrameConstantBuffers(aScene);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);
	mpDeviceContext->PSSetShader(tPS->GetPixelShader(), NULL, 0);


	mpDeviceContext->PSSetSamplers(0, 1, mpAnisotropicWrapSampler.GetAddressOf());
	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());


	for (int i = 0; i < aScene->mObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aScene->mObjects[i].get(), aScene);
		RenderObject(aScene->mObjects[i].get());
	}

	

	// Present the rendered scene to the screen.
}

void Renderer::RenderObject(IObject* const aObject)
{
	// Bind vertex/index buffers
	aObject->mpModel->Render(mpDeviceContext.Get());

	int indices = aObject->mpModel->GetIndexCount();

	// Bind textures from material
	d3dMaterial* const aMaterial = aObject->mpModel->mMaterial.get();

	ID3D11ShaderResourceView* aView = aMaterial->mpDiffuse->GetTexture();
	mpDeviceContext->PSSetShaderResources(0, 1, &aView);

	ID3D11ShaderResourceView* aView2 = aMaterial->mpSpecular->GetTexture();
	mpDeviceContext->PSSetShaderResources(1, 1, &aView2);

	ID3D11ShaderResourceView* aView3 = aMaterial->mpNormal->GetTexture();
	mpDeviceContext->PSSetShaderResources(2, 1, &aView3);


	// Render the model.
	mpDeviceContext->DrawIndexed(indices, 0, 0);
	
}

bool Renderer::InitializeDeviceAndContext()
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
		return false;
	}
	return true;
}

bool Renderer::InitializeSwapchain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = GraphicsSettings::gCurrentScreenWidth;
	swapChainDesc.BufferDesc.Height = GraphicsSettings::gCurrentScreenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (GraphicsSettings::gIsVsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = gnumerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = gdenominator;
	}

	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = windowHandle;

	// Turn multi-sampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (GraphicsSettings::gIsApplicationFullScreen)
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

	HRESULT result;
	IDXGISwapChain* tSC = mpSwapchain.Get();
	result = mFactory->CreateSwapChain(mpDevice.Get(), &swapChainDesc, &mpSwapchain);


	if (FAILED(result))
	{
		LOG(INFO) << "Failed to create swapchain";
	}
	return true;
}

bool Renderer::InitializeDXGI()
{
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	HRESULT result;
	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&mFactory);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to create DXGIFactory";
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = mFactory->EnumAdapters(0, &mAdapter);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to enumerate adapters";
		return false;
	}

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
		if (displayModeList[i].Width == (unsigned int)GraphicsSettings::gCurrentScreenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)GraphicsSettings::gCurrentScreenHeight)
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
	gVideoCardMemoryAmount = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);


	stringLength = 0;
	size_t lValue = (size_t)stringLength;

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&lValue, gVideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	return true;
}

bool Renderer::InitializeBackBuffRTV()
{
	HRESULT result;

	ID3D11Texture2D* backBufferPtr;
	result = mpSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
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

	// Release the backbuffer ptr
	backBufferPtr->Release();
	backBufferPtr = 0;

	return true;
}

bool  Renderer::InitializeDepthStencilView()
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	HRESULT result;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = GraphicsSettings::gCurrentScreenWidth;
	depthBufferDesc.Height = GraphicsSettings::gCurrentScreenHeight;
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
	result = mpDevice->CreateTexture2D(&depthBufferDesc, NULL, &mpDepthStencilBufferTexture);
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
	result = mpDevice->CreateDepthStencilState(&depthStencilDesc, &mpDepthStencilState);
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

	result = mpDevice->CreateDepthStencilView(mpDepthStencilBufferTexture.Get(), &depthStencilViewDesc, &mpDepthStencilView);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil view";
	}
	return true;
}

bool  Renderer::InitializeRasterstate()
{
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

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

	return true;
}

bool Renderer::DestroyDirectX()
{
	if (mpDevice.Get() != nullptr)
	{
		mpDevice.Reset();
		mpDevice = nullptr;
	}

	if (mpDeviceContext.Get() != nullptr)
	{
		mpDeviceContext.Reset();
		mpDeviceContext = nullptr;
	}

	if (mpRenderTargetView.Get() != nullptr)
	{
		mpRenderTargetView.Reset();
		mpRenderTargetView = nullptr;
	}

	return true;
}

bool  Renderer::InitializeSamplerState()
{
	//ID3D10Blob* errorMessage;
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
	HRESULT result = mpDevice->CreateSamplerState(&samplerDesc, &mpAnisotropicWrapSampler);
	if (FAILED(result))
	{
		LOG(WARNING) << "Failed to sampler states";
		return false;
	}
	return true;
}

bool Renderer::InitializeViewportAndMatrices()
{
	gViewPort.Width = (float)GraphicsSettings::gCurrentScreenWidth;
	gViewPort.Height = (float)GraphicsSettings::gCurrentScreenHeight;
	gViewPort.MinDepth = 0.0f;
	gViewPort.MaxDepth = 1.0f;
	gViewPort.TopLeftX = 0.0f;
	gViewPort.TopLeftY = 0.0f;

	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)GraphicsSettings::gCurrentScreenWidth / (float)GraphicsSettings::gCurrentScreenHeight;
	XMStoreFloat4x4(&gProjectionMatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_FAR));

	XMFLOAT3 upF3, pF3, LF3;
	XMVECTOR upVector, positionVector, lookAtVector;

	upF3.x = 0; upF3.y = 1; upF3.z = 0;
	pF3.x = 0; pF3.y = 1.0; pF3.z = -4;
	LF3.x = 0; LF3.y = 0; LF3.z = 0;

	upVector = XMLoadFloat3(&upF3);
	positionVector = XMLoadFloat3(&pF3);
	lookAtVector = XMLoadFloat3(&LF3);


	XMStoreFloat4x4(&gViewMatrix, XMMatrixLookAtLH(positionVector, lookAtVector, upVector));

	return true;
}

bool Renderer::InitializeDirectX()
{
	if (!InitializeDeviceAndContext())
	{
		LOG(INFO) << "Failed to create device and/or context";
		return false;
	}
	LOG(INFO) << "Successfully initialized device and context";

	if (!InitializeDXGI())
	{
		LOG(INFO) << "Failed to create DXGI";
		return false;
	}
	LOG(INFO) << "Successfully initialized DXGI";

	if (!InitializeSwapchain())
	{
		LOG(INFO) << "Failed to create Swapchain";
		return false;
	}
	LOG(INFO) << "Successfully initialized Swapchain";

	if (!InitializeBackBuffRTV())
	{
		LOG(INFO) << "Failed to create RTV with back buffer";
		return false;
	}
	LOG(INFO) << "Successfully initialized backbuffer rtv";

	if (!InitializeDepthStencilView())
	{
		LOG(INFO) << "failed to create DSV";
		return false;
	}
	LOG(INFO) << "Succesfully initialized dsv";

	if (!InitializeRasterstate())
	{
		LOG(INFO) << "Failed to create raster state";
		return false;
	}
	LOG(INFO) << "Succesfully initialized raster state";

	if (!InitializeSamplerState())
	{
		LOG(INFO) << "Failed to create raster state";
		return false;
	}
	LOG(INFO) << "Succesfully initialized raster state";

	if (!InitializeViewportAndMatrices())
	{
		LOG(INFO) << "Failed to create viewport";
		return false;
	}
	LOG(INFO) << "Successfully initialized viewp and matrices";

	LOG(INFO) << "Successfully initialized DirectX!";

	return true;
}
