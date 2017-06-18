#include "Renderer.h"

#include "easylogging++.h"
#include "Imgui.h"

#include "IScene.h"
#include "d3dConstantBuffer.h"
#include "d3dShaderManager.h"
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"
#include "d3dTexture.h"
#include "d3dRenderTexture.h"
#include "d3dMaterial.h"

#include "ResourceManager.h"
#include "ConstantBuffers.h"
#include "IObject.h"
#include "GraphicsSettings.h"
#include "camera.h"


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
	mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(cbMatrixBuffer), nullptr, mpDevice.Get());
	mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(cbMaterial), nullptr, mpDevice.Get());
	mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(cbLights), nullptr, mpDevice.Get());
	mpPerObjectCB = std::make_unique<d3dConstantBuffer>(sizeof(cbPerObject), nullptr, mpDevice.Get());
	mpLightMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(cbLightMatrix), nullptr, mpDevice.Get());
	LOG(INFO) << "Constant buffers created";
}


// Data ptrs for constant buffer data
static std::unique_ptr<cbLights> gLightBufferDataPtr = std::make_unique<cbLights>();
static std::unique_ptr<cbMaterial> gMaterialBufferDataPtr = std::make_unique<cbMaterial>();
static std::unique_ptr<cbMatrixBuffer> gMatrixBufferDataPtr = std::make_unique<cbMatrixBuffer>();
static std::unique_ptr<cbLightMatrix> gLightMatrixBufferDataPtr = std::make_unique<cbLightMatrix>();
static std::unique_ptr<cbPerObject> gPerObjectMatrixBufferDataPtr = std::make_unique<cbPerObject>();


void Renderer::UpdateFrameConstantBuffers(std::vector <std::unique_ptr<Light>>& apLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera)
{

	// Update light constant buffers
	uint32_t bufferNumber;

	gLightBufferDataPtr->amountOfLights = static_cast<int>(apLights.size());

	for (int i = 0; i < apLights.size(); ++i)
	{
		gLightBufferDataPtr->arr[i].position = apLights[i]->position;
		gLightBufferDataPtr->arr[i].diffuseColor = apLights[i]->diffuseColor;
	}

	gLightBufferDataPtr->directionalLight.diffuseColor = aDirectionalLight->mDiffuseColor;
	gLightBufferDataPtr->directionalLight.specularColor = aDirectionalLight->mSpecularColor;
	gLightBufferDataPtr->directionalLight.position = aDirectionalLight->mDirectionVector;


	mpLightCB->UpdateBuffer((void*)gLightBufferDataPtr.get(), mpDeviceContext.Get());

	bufferNumber = 2;
	ID3D11Buffer* tBuff = mpLightCB->GetBuffer();

	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);


	XMMATRIX viewMatrix, projectionMatrix;
	projectionMatrix = apCamera->GetProj();
	viewMatrix = apCamera->GetView();


	// Transpose the matrices to prepare them for the shader.
	XMMATRIX viewMatrix2 = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrix2 = XMMatrixTranspose(projectionMatrix);

	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	gMatrixBufferDataPtr->view = viewMatrix2;
	gMatrixBufferDataPtr->projection = projectionMatrix2;
	gMatrixBufferDataPtr->gEyePosX = apCamera->GetPosition3f().x;
	gMatrixBufferDataPtr->gEyePosY = apCamera->GetPosition3f().y;
	gMatrixBufferDataPtr->gEyePosZ = apCamera->GetPosition3f().z;


	mpMatrixCB->UpdateBuffer((void*)gMatrixBufferDataPtr.get(), mpDeviceContext.Get());

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	tBuff = mpMatrixCB->GetBuffer();
	// finally set the constant buffer in the vertex shader with the updated values.
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

	UpdateShadowLightConstantBuffers(aDirectionalLight);
}


void Renderer::UpdateShadowLightConstantBuffers(d3dLightClass* const aDirectionalLight)
{
	uint32_t bufferNumber = 3;
	XMMATRIX viewMatrix, projectionMatrix;
	aDirectionalLight->GetViewMatrix(viewMatrix);
	aDirectionalLight->GetProjectionMatrix(projectionMatrix);

	gLightMatrixBufferDataPtr->lightViewMatrix = XMMatrixTranspose(viewMatrix);
	gLightMatrixBufferDataPtr->lightProjectionMatrix = XMMatrixTranspose(projectionMatrix);

	this->mpLightMatrixCB->UpdateBuffer((void*)gLightMatrixBufferDataPtr.get(), mpDeviceContext.Get());

	ID3D11Buffer* tBuff = mpLightMatrixCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
}


void Renderer::UpdateObjectConstantBuffers(IObject* const aObject)
{
	uint32_t bufferNumber;

	// Update material
	d3dMaterial* const tMat = aObject->mpModel->mMaterial.get();

	gMaterialBufferDataPtr->hasDiffuse = (int)tMat->mpDiffuse->exists;
	gMaterialBufferDataPtr->hasSpecular = (int)tMat->mpSpecular->exists;
	gMaterialBufferDataPtr->hasNormal = (int)tMat->mpNormal->exists;


	mpMaterialCB->UpdateBuffer((void*)gMaterialBufferDataPtr.get(), mpDeviceContext.Get());
	bufferNumber = 1;
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
	// End update material


	XMMATRIX worldMatrix;
	worldMatrix = XMLoadFloat4x4(&aObject->mWorldMatrix);

	XMMATRIX worldMatrix2 = XMMatrixTranspose(worldMatrix);
	gPerObjectMatrixBufferDataPtr->worldMatrix = worldMatrix2;
	
	mpPerObjectCB->UpdateBuffer((void*)gPerObjectMatrixBufferDataPtr.get(), mpDeviceContext.Get());

	bufferNumber = 4;

	tBuff = mpPerObjectCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
}


void Renderer::RenderScene(
	std::vector<std::unique_ptr<IObject>>& aObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	d3dLightClass* const aDirectionalLight,
	Camera* const apCamera
)
{
	float color[4]{ clearColor[0], clearColor[1], clearColor[2], 1.0f };

	// Update frame constant buffers  
	UpdateFrameConstantBuffers(aLights, aDirectionalLight, apCamera);

	RenderSceneDepthPrePass(aObjects);
	
	// Scene prep
	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull.Get());

	mpDeviceContext->ClearRenderTargetView(this->mPostProcColorBuffer->rtv, clearColor);
	mpDeviceContext->ClearDepthStencilView(this->mPostProcDepthBuffer->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState.Get(), 1);
	mpDeviceContext->OMSetRenderTargets(1, &mPostProcColorBuffer->rtv, this->mPostProcDepthBuffer->dsv);

	apCamera->UpdateViewMatrix();

	d3dShaderVS*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_shadow.hlsl");
	d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_shadow.hlsl");


	// Set samplers
	mpDeviceContext->PSSetSamplers(0, 1, mpPointClampSampler.GetAddressOf());
	mpDeviceContext->PSSetSamplers(1, 1, mpLinearClampSampler.GetAddressOf());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);
	mpDeviceContext->PSSetShader(tPS->GetPixelShader(), NULL, 0);

	
	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());

	ID3D11ShaderResourceView* aView = mShadowDepthBuffer->srv;
	mpDeviceContext->PSSetShaderResources(3, 1, &aView);

	// Render objects
	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i].get());
		RenderObject(aObjects[i].get());
	}

	// Render post processing quad
	RenderFullScreenQuad();
	ImGui::Render();

	mpDeviceContext->ClearState();
}


// Render the scene to a depth map texture
void Renderer::RenderSceneDepthPrePass(std::vector<std::unique_ptr<IObject>>& aObjects)
{
	mpDeviceContext->RSSetViewports(1, &mShadowLightViewport);
	mpDeviceContext->RSSetState(mRaster_backcull.Get());

	mpDeviceContext->ClearDepthStencilView(this->mShadowDepthBuffer->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState.Get(), 1);
	mpDeviceContext->OMSetRenderTargets(0, nullptr, mShadowDepthBuffer->dsv);

	d3dShaderVS*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_depth.hlsl");
	//d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_depth.hlsl");

	//UpdateFrameConstantBuffers(aScene);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);

	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());


	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i].get());
		RenderObject(aObjects[i].get());
	}
}


// Render scene to full screen quad
void Renderer::RenderFullScreenQuad()
{
	// Get the fullscreen shaders
	d3dShaderVS*const tFVS = mpShaderManager->GetVertexShader("Shaders\\fullScreenQuad_VS.hlsl");
	d3dShaderPS*const tFPS = mpShaderManager->GetPixelShader("Shaders\\fullScreenQuad_PS.hlsl");


	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull.Get());

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState.Get(), 1);
	mpDeviceContext->OMSetRenderTargets(1, &this->mBackBufferTexture->rtv, mBackBufferTexture->dsv);

	mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Draw full screen quad
	mpDeviceContext->PSSetSamplers(0, 1, this->mpPointClampSampler.GetAddressOf());
	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->GetVertexShader(), NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->GetPixelShader(), NULL, 0);
	mpDeviceContext->PSSetShaderResources(0, 1, &mPostProcColorBuffer->srv);
	mpDeviceContext->Draw(4, 0);
}

void Renderer::RenderMaterial(d3dMaterial* const aMaterial)
{
	// Bind textures from material
	ID3D11ShaderResourceView* aView = aMaterial->mpDiffuse->GetTexture();
	mpDeviceContext->PSSetShaderResources(0, 1, &aView);

	ID3D11ShaderResourceView* aView2 = aMaterial->mpSpecular->GetTexture();
	mpDeviceContext->PSSetShaderResources(1, 1, &aView2);

	ID3D11ShaderResourceView* aView3 = aMaterial->mpNormal->GetTexture();
	mpDeviceContext->PSSetShaderResources(2, 1, &aView3);
}

// Render the object
void Renderer::RenderObject(IObject* const aObject)
{
	// Bind vertex/index buffers
	aObject->mpModel->Render(mpDeviceContext.Get());

	int indices = aObject->mpModel->GetIndexCount();

	// Bind textures from material
	d3dMaterial* const aMaterial = aObject->mpModel->mMaterial.get();
	
	if (aMaterial != nullptr)
	{
		RenderMaterial(aMaterial);
	}

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


// Initialize swap chain
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
	//mSceneRenderTexture = std::make_unique<d3dRenderTexture>();
	//mSceneRenderTexture->Initialize(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, SCREEN_NEAR, SCREEN_FAR);

	mBackBufferTexture = std::make_unique<Texture>();
	mPostProcColorBuffer = std::make_unique<Texture>();
	mPostProcDepthBuffer = std::make_unique<Texture>();
	mShadowDepthBuffer = std::make_unique<Texture>();

	mPostProcColorBuffer->texture = CreateSimpleRenderTargetTexture(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight);
	mPostProcColorBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), mPostProcColorBuffer->texture);
	mPostProcColorBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mPostProcColorBuffer->texture);
	
	mPostProcDepthBuffer->texture = CreateSimpleDepthTexture(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight);
	mPostProcDepthBuffer->dsv = CreateSimpleDepthBuffer(mpDevice.Get(), mPostProcDepthBuffer->texture);

	mBackBufferTexture->texture = CreateSimpleDepthTexture(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight);
	mBackBufferTexture->dsv =  CreateSimpleDepthstencilView(mpDevice.Get(), mBackBufferTexture->texture);
	mBackBufferTexture->rtv = CreateRenderTargetViewFromSwapchain(mpDevice.Get(), mpSwapchain.Get());

	mShadowDepthBuffer->texture = CreateSimpleDepthTextureVisibleShader(mpDevice.Get(), 1024, 1024);
	mShadowDepthBuffer->dsv = CreateSimpleDepthBuffer(mpDevice.Get(), mShadowDepthBuffer->texture);
	mShadowDepthBuffer->srv = CreateSRVTex2DDepth(mpDevice.Get(), mShadowDepthBuffer->texture);
	return true;
}

#include "d3d11HelperFile.h"
bool  Renderer::InitializeDepthStencilView()
{
	mpDepthStencilState = CreateDepthStateDefault(mpDevice.Get());

	//this->mSceneDepthPrepassTexture = std::make_unique<d3dRenderDepthTexture>();
	//this->mSceneDepthPrepassTexture->Initialize(mpDevice.Get(), 2048, 2048, SCREEN_NEAR, SCREEN_FAR);

	return true;
}


bool  Renderer::InitializeRasterstate()
{
	this->mRaster_backcull = CreateRSDefault(mpDevice.Get());
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

	return true;
}


bool  Renderer::InitializeSamplerState()
{
	mpAnisotropicWrapSampler = CreateSamplerAnisotropicWrap(mpDevice.Get());
	mpPointClampSampler = CreateSamplerPointClamp(mpDevice.Get());
	mpLinearClampSampler = CreateSamplerLinearClamp(mpDevice.Get());
	
	return true;
}


bool Renderer::InitializeViewportAndMatrices()
{
	mShadowLightViewport.Width = 2048.0f;
	mShadowLightViewport.Height = 2048.0f;
	mShadowLightViewport.MinDepth = 0.0f;
	mShadowLightViewport.MaxDepth = 1.0f;
	mShadowLightViewport.TopLeftX = 0.0f;
	mShadowLightViewport.TopLeftY = 0.0f;

	mViewport.Width = (float)GraphicsSettings::gCurrentScreenWidth;
	mViewport.Height = (float)GraphicsSettings::gCurrentScreenHeight;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;

	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)GraphicsSettings::gCurrentScreenWidth / (float)GraphicsSettings::gCurrentScreenHeight;
	XMStoreFloat4x4(&mProjectionMatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_FAR));

	XMFLOAT3 upF3, pF3, LF3;
	XMVECTOR upVector, positionVector, lookAtVector;

	upF3.x = 0; upF3.y = 1; upF3.z = 0;
	pF3.x = 0; pF3.y = 1.0; pF3.z = -4;
	LF3.x = 0; LF3.y = 0; LF3.z = 0;

	upVector = XMLoadFloat3(&upF3);
	positionVector = XMLoadFloat3(&pF3);
	lookAtVector = XMLoadFloat3(&LF3);


	XMStoreFloat4x4(&mViewMatrix, XMMatrixLookAtLH(positionVector, lookAtVector, upVector));

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
	LOG(INFO) << "Successfully initialized dsv";

	if (!InitializeRasterstate())
	{
		LOG(INFO) << "Failed to create raster state";
		return false;
	}
	LOG(INFO) << "Successfully initialized raster state";

	if (!InitializeSamplerState())
	{
		LOG(INFO) << "Failed to create raster state";
		return false;
	}
	LOG(INFO) << "Successfully initialized raster state";

	if (!InitializeViewportAndMatrices())
	{
		LOG(INFO) << "Failed to create viewport";
		return false;
	}
	LOG(INFO) << "Successfully initialized viewp and matrices";

	LOG(INFO) << "Successfully initialized DirectX!";

	return true;
}
