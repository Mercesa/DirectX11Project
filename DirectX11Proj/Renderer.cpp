#include "Renderer.h"

#include <chrono>

#include "Imgui.h"

#include "IScene.h"
#include "d3dConstantBuffer.h"
#include "d3dShaderManager.h"
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"


#include "ResourceManager.h"
#include "ConstantBuffers.h"
#include "IObject.h"
#include "camera.h"
#include "GraphicsStructures.h"
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
	ResourceManager::GetInstance().mpDevice = this->mpDevice.Get();
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


	UpdateShadowLightConstantBuffers(aDirectionalLight);
}


void Renderer::UpdateShadowLightConstantBuffers(d3dLightClass* const aDirectionalLight)
{
	XMMATRIX viewMatrix, projectionMatrix;
	aDirectionalLight->GetViewMatrix(viewMatrix);
	aDirectionalLight->GetProjectionMatrix(projectionMatrix);

	gLightMatrixBufferDataPtr->lightViewMatrix = XMMatrixTranspose(viewMatrix);
	gLightMatrixBufferDataPtr->lightProjectionMatrix = XMMatrixTranspose(projectionMatrix);

	this->mpLightMatrixCB->UpdateBuffer((void*)gLightMatrixBufferDataPtr.get(), mpDeviceContext.Get());
}


void Renderer::UpdateObjectConstantBuffers(IObject* const aObject)
{
	ResourceManager& rm = ResourceManager::GetInstance();
	
	// Update material
	Model* const tModel = ResourceManager::GetInstance().GetModelByID(aObject->mpModel);
	d3dMaterial* const tMat = tModel->material;

	// Check if texture exists or not
	gMaterialBufferDataPtr->hasDiffuse = (int)(rm.GetTextureByID(tMat->mpDiffuse)->srv == nullptr ? false : true);
	gMaterialBufferDataPtr->hasSpecular = (int)(rm.GetTextureByID(tMat->mpSpecular)->srv == nullptr ? false : true);
	gMaterialBufferDataPtr->hasNormal = (int)(rm.GetTextureByID(tMat->mpNormal)->srv == nullptr ? false : true);



	mpMaterialCB->UpdateBuffer((void*)gMaterialBufferDataPtr.get(), mpDeviceContext.Get());


	XMMATRIX worldMatrix2 = XMMatrixTranspose(XMLoadFloat4x4(&aObject->mWorldMatrix));
	gPerObjectMatrixBufferDataPtr->worldMatrix = worldMatrix2;
	
	mpPerObjectCB->UpdateBuffer((void*)gPerObjectMatrixBufferDataPtr.get(), mpDeviceContext.Get());

	
}


static float objectRenderingTime = 0.0f;
void Renderer::RenderSceneWithShadows(std::vector<std::unique_ptr<IObject>>& aObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	d3dLightClass* const aDirectionalLight,
	Camera* const apCamera)
{
	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->ClearRenderTargetView(this->mPostProcColorBuffer->rtv, clearColor);
	mpDeviceContext->ClearDepthStencilView(this->mPostProcDepthBuffer->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(1, &mPostProcColorBuffer->rtv, this->mPostProcDepthBuffer->dsv);

	apCamera->UpdateViewMatrix();

	d3dShaderVS*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_shadow.hlsl");
	d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_shadow.hlsl");


	// Set samplers
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpLinearClampSampler);
	mpDeviceContext->PSSetSamplers(2, 1, &mpAnisotropicWrapSampler);
	mpDeviceContext->PSSetSamplers(3, 1, &mpLinearWrapSampler);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);
	mpDeviceContext->PSSetShader(tPS->GetPixelShader(), NULL, 0);


	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());

	ID3D11ShaderResourceView* aView = mShadowDepthBuffer->srv;
	mpDeviceContext->PSSetShaderResources(3, 1, &aView);


	auto renderSceneObjectsStart = std::chrono::high_resolution_clock::now();
	
	uint32_t bufferNumber;
	bufferNumber = 1;
	
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

	bufferNumber = 4;
	tBuff = mpPerObjectCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
	
	// Render objects
	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i].get());
		RenderObject(aObjects[i].get());
	}
	auto renderSceneObjectsEnd = std::chrono::high_resolution_clock::now();
	objectRenderingTime = std::chrono::duration_cast<std::chrono::microseconds>(renderSceneObjectsEnd - renderSceneObjectsStart).count() / 1000.0f;


}


void Renderer::RenderScene(
	std::vector<std::unique_ptr<IObject>>& aObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	d3dLightClass* const aDirectionalLight,
	Camera* const apCamera
)
{
	
	RenderSceneForward(aObjects, aLights, aDirectionalLight, apCamera);
}


void Renderer::RenderSceneForward(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera)
{
	float color[4]{ clearColor[0], clearColor[1], clearColor[2], 1.0f };

	// Update frame constant buffers  


	// Update and bind constant buffers
	auto frameConstantBufferTimerStart = std::chrono::high_resolution_clock::now();
	UpdateFrameConstantBuffers(aLights, aDirectionalLight, apCamera);

	ID3D11Buffer* tBuff = mpLightMatrixCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(3, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(3, 1, &tBuff);

	tBuff = mpLightCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(2, 1, &tBuff);

	tBuff = mpMatrixCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(0, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(0, 1, &tBuff);

	auto frameConstantBufferTimerEnd = std::chrono::high_resolution_clock::now();


	auto renderSceneDepthPrePassTimerStart = std::chrono::high_resolution_clock::now();
	RenderSceneDepthPrePass(aObjects);
	auto renderSceneDepthPrePassTimerEnd = std::chrono::high_resolution_clock::now();

	auto renderSceneWithShadowsTimerStart = std::chrono::high_resolution_clock::now();
	RenderSceneWithShadows(aObjects, aLights, aDirectionalLight, apCamera);
	auto renderSceneWithShadowsTimerEnd = std::chrono::high_resolution_clock::now();

	// Scene prep

	// Render post processing quad
	RenderFullScreenQuad();

	static int frames = 0;
	++frames;

	static float fCbtiming = 0.0f;
	static float depthPrePassTiming = 0.0f;
	static float shadowSceneTiming = 0.0f;
	static float shadowSceneObjectTiming = 0.0f;
	if (frames % 15 == 0)
	{
		fCbtiming = std::chrono::duration_cast<std::chrono::microseconds>(frameConstantBufferTimerEnd - frameConstantBufferTimerStart).count() / 1000.0f;
		depthPrePassTiming = std::chrono::duration_cast<std::chrono::microseconds>(renderSceneDepthPrePassTimerEnd - renderSceneDepthPrePassTimerStart).count() / 1000.0f;
		shadowSceneTiming = std::chrono::duration_cast<std::chrono::microseconds>(renderSceneWithShadowsTimerEnd - renderSceneWithShadowsTimerStart).count() / 1000.0f;
		shadowSceneObjectTiming = objectRenderingTime;
	}

	ImGui::Text("Updating frame constant %.5f ms/frame", fCbtiming);
	ImGui::Text("Render scene depth pre-pass %.5f ms/frame", depthPrePassTiming);
	ImGui::Text("Render scene with shadows %.5f ms/frame", shadowSceneTiming);
	ImGui::Text("Render shadow scene objects %.5f ms/frame", shadowSceneObjectTiming);
}


// Render the scene to a depth map texture
void Renderer::RenderSceneDepthPrePass(std::vector<std::unique_ptr<IObject>>& aObjects)
{
	mpDeviceContext->RSSetViewports(1, &mShadowLightViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->ClearDepthStencilView(this->mShadowDepthBuffer->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(0, nullptr, mShadowDepthBuffer->dsv);

	d3dShaderVS*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_depth.hlsl");
	//d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_depth.hlsl");

	//UpdateFrameConstantBuffers(aScene);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);

	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());

	uint32_t bufferNumber;
	bufferNumber = 1;

	// bind buffers
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

	bufferNumber = 4;
	tBuff = mpPerObjectCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

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
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(1, &this->mBackBufferTexture->rtv, mBackBufferTexture->dsv);

	mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Draw full screen quad
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->GetVertexShader(), NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->GetPixelShader(), NULL, 0);
	mpDeviceContext->PSSetShaderResources(0, 1, &mPostProcColorBuffer->srv);
	mpDeviceContext->Draw(4, 0);
}

void Renderer::RenderMaterial(d3dMaterial* const aMaterial)
{
	// Bind textures from material
	ID3D11ShaderResourceView* aView = ResourceManager::GetInstance().GetTextureByID(aMaterial->mpDiffuse)->srv;
	mpDeviceContext->PSSetShaderResources(0, 1, &aView);

	ID3D11ShaderResourceView* aView2 = ResourceManager::GetInstance().GetTextureByID(aMaterial->mpSpecular)->srv;
	mpDeviceContext->PSSetShaderResources(1, 1, &aView2);

	ID3D11ShaderResourceView* aView3 = ResourceManager::GetInstance().GetTextureByID(aMaterial->mpNormal)->srv;
	mpDeviceContext->PSSetShaderResources(2, 1, &aView3);
}

// Render the object
void Renderer::RenderObject(IObject* const aObject)
{
	Model* const model = ResourceManager::GetInstance().GetModelByID(aObject->mpModel);
	// Bind vertex/index buffers
	RenderBuffers(mpDeviceContext.Get(), model);

	uint32_t indices = (uint32_t)model->indexBuffer->amountOfElements;

	// Bind textures from material
	d3dMaterial* const aMaterial = model->material;
	
	if (aMaterial != nullptr)
	{
		RenderMaterial(aMaterial);
	}

	// Render the model.
	mpDeviceContext->DrawIndexed(indices, 0, 0);
}

void Renderer::RenderBuffers(ID3D11DeviceContext* deviceContext, Model* const aModel)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexData);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.

	ID3D11Buffer* tBuffer = aModel->vertexBuffer->buffer;

	deviceContext->IASetVertexBuffers(0, 1, &tBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(aModel->indexBuffer->buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
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

	featureLevel = D3D_FEATURE_LEVEL_11_1;
	
	
	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, 0, 0, D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext);
	mpDeviceContext.As(&mpDeviceContext1);
	mpDevice.As(&mpDevice1);


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

	gBuffer_albedoBuffer = std::make_unique<Texture>();
	gBuffer_normalBuffer = std::make_unique<Texture>();
	gBuffer_specularBuffer = std::make_unique<Texture>();

	// Post proc color and depth buffer 
	mPostProcColorBuffer->texture =  CreateSimpleRenderTargetTexture(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	mPostProcColorBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), mPostProcColorBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mPostProcColorBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mPostProcColorBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	
	mPostProcDepthBuffer->texture = CreateSimpleDepthTexture(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	mPostProcDepthBuffer->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), mPostProcDepthBuffer->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);

	// Backbuffer
	mBackBufferTexture->texture = CreateSimpleDepthTexture(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	mBackBufferTexture->dsv =  CreateSimpleDepthstencilView(mpDevice.Get(), mBackBufferTexture->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);
	mBackBufferTexture->rtv = CreateRenderTargetViewFromSwapchain(mpDevice.Get(), mpSwapchain.Get());

	// Buffer for shadow mapping
	mShadowDepthBuffer->texture = CreateSimpleDepthTexture(mpDevice.Get(), 8096, 8096, GetDepthResourceFormat(DXGI_FORMAT_D24_UNORM_S8_UINT), D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	mShadowDepthBuffer->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), mShadowDepthBuffer->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);
	mShadowDepthBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mShadowDepthBuffer->texture, GetDepthSRVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT));



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
	ReleaseTexture(mBackBufferTexture.get());
	
	ReleaseTexture(mPostProcColorBuffer.get());
	ReleaseTexture(mPostProcDepthBuffer.get());
	
	ReleaseTexture(mShadowDepthBuffer.get());

	ReleaseTexture(gBuffer_albedoBuffer.get());
	ReleaseTexture(gBuffer_normalBuffer.get());
	ReleaseTexture(gBuffer_specularBuffer.get());


	mRaster_backcull->Release();
	mpDepthStencilState->Release();
	mpAnisotropicWrapSampler->Release();
	mpLinearClampSampler->Release();
	mpPointClampSampler->Release();
	mpLinearWrapSampler->Release();

	return true;
}


bool  Renderer::InitializeSamplerState()
{
	mpAnisotropicWrapSampler = CreateSamplerAnisotropicWrap(mpDevice.Get());
	mpPointClampSampler = CreateSamplerPointClamp(mpDevice.Get());
	mpLinearClampSampler = CreateSamplerLinearClamp(mpDevice.Get());
	mpLinearWrapSampler = CreateSamplerLinearWrap(mpDevice.Get());
	return true;
}


bool Renderer::InitializeViewportAndMatrices()
{
	mShadowLightViewport.Width = 8096.0f;
	mShadowLightViewport.Height = 8096.0f;
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
