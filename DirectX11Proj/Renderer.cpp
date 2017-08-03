#include "Renderer.h"

#include <chrono>

#include "Imgui.h"

#include "IScene.h"
#include "d3dConstantBuffer.h"
#include "d3dShaderManager.h"


#include "ResourceManager.h"
#include "ConstantBuffers.h"
#include "IObject.h"
#include "camera.h"
#include "GraphicsStructures.h"
#include "GraphicsSettings.h"

#include <random>

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
	mpBlurCB = std::make_unique<d3dConstantBuffer>(sizeof(cbBlurParameters), nullptr, mpDevice.Get());

	LOG(INFO) << "Constant buffers created";
}


// Data ptrs for constant buffer data
static std::unique_ptr<cbLights> gLightBufferDataPtr = std::make_unique<cbLights>();
static std::unique_ptr<cbMaterial> gMaterialBufferDataPtr = std::make_unique<cbMaterial>();
static std::unique_ptr<cbMatrixBuffer> gMatrixBufferDataPtr = std::make_unique<cbMatrixBuffer>();
static std::unique_ptr<cbLightMatrix> gLightMatrixBufferDataPtr = std::make_unique<cbLightMatrix>();
static std::unique_ptr<cbPerObject> gPerObjectMatrixBufferDataPtr = std::make_unique<cbPerObject>();
static std::unique_ptr<cbBlurParameters> gBlurParamatersDataPtr = std::make_unique<cbBlurParameters>();


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
	XMMATRIX invView = XMMatrixTranspose((XMMatrixInverse(nullptr, apCamera->GetView())));

	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	gMatrixBufferDataPtr->view = viewMatrix2;
	gMatrixBufferDataPtr->projection = projectionMatrix2;
	gMatrixBufferDataPtr->viewMatrixInversed = invView;
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
	Material* const tMat = tModel->material;

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

	VertexShader*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_shadow.hlsl");
	PixelShader*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_shadow.hlsl");


	// Set samplers
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpLinearClampSampler);
	mpDeviceContext->PSSetSamplers(2, 1, &mpAnisotropicWrapSampler);
	mpDeviceContext->PSSetSamplers(3, 1, &mpLinearWrapSampler);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tPS->shader, NULL, 0);


	mpDeviceContext->IASetInputLayout(tVS->inputLayout);

	ID3D11ShaderResourceView* aView = mShadowDepthBuffer->srv;
	mpDeviceContext->PSSetShaderResources(3, 1, &aView);


	auto renderSceneObjectsStart = std::chrono::high_resolution_clock::now();
	
	// Render objects
	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i].get());
		RenderObject(aObjects[i].get());
	}
	auto renderSceneObjectsEnd = std::chrono::high_resolution_clock::now();
	objectRenderingTime = std::chrono::duration_cast<std::chrono::microseconds>(renderSceneObjectsEnd - renderSceneObjectsStart).count() / 1000.0f;

}

void Renderer::RenderSceneSkybox(IObject* const aObject)
{
	mpDeviceContext->RSSetState(mRaster_noCull);
	mpDeviceContext->OMSetDepthStencilState(mDepthStencilStateLessEqual, 1);

	mpDeviceContext->OMSetRenderTargets(1, &mPostProcColorBuffer->rtv, this->mPostProcDepthBuffer->dsv);

	VertexShader*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_Skybox.hlsl");
	PixelShader *const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_Skybox.hlsl");

	mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tPS->shader, NULL, 0);

	mpDeviceContext->IASetInputLayout(tVS->inputLayout);
	UpdateObjectConstantBuffers(aObject);
	RenderObject(aObject); 
	

}

static bool renderForward = true;


void Renderer::RenderScene(
	std::vector<std::unique_ptr<IObject>>& aObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	d3dLightClass* const aDirectionalLight,
	Camera* const apCamera,
	IObject* const aSkybox
)
{
	ImGui::MenuItem("Enabled", NULL, &renderForward);

	if(renderForward)
	{
		RenderSceneForward(aObjects, aLights, aDirectionalLight, apCamera, aSkybox);
	}

	else
	{
		RenderSceneDeferred(aObjects, aLights, aDirectionalLight, apCamera);
	}
}

void Renderer::RenderSceneGBufferFill(std::vector<std::unique_ptr<IObject>>& aObjects)
{
	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);
	
	mpDeviceContext->ClearDepthStencilView(this->gBuffer_depthBuffer->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	ID3D11RenderTargetView* renderTargetArray[3] = { gBuffer_albedoBuffer->rtv, gBuffer_normalBuffer->rtv, gBuffer_positionBuffer->rtv };

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(3, renderTargetArray, gBuffer_depthBuffer->dsv);
	
	VertexShader *const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_GBufferFill.hlsl");
	PixelShader*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_GBufferFill.hlsl");
	
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpLinearClampSampler);
	mpDeviceContext->PSSetSamplers(2, 1, &mpAnisotropicWrapSampler);
	mpDeviceContext->PSSetSamplers(3, 1, &mpLinearWrapSampler);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tPS->shader, NULL, 0);

	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->inputLayout);
	
	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i].get());
		RenderObject(aObjects[i].get());
	}
}

void Renderer::RenderBlurPass()
{
	VertexShader*const tFVS = mpShaderManager->GetVertexShader("Shaders\\VS_Blur.hlsl");
	PixelShader*const tFPS = mpShaderManager->GetPixelShader("Shaders\\PS_Blur.hlsl");


	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);



	// Draw full screen quad
	//mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpPointWrapSampler);

	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);

	// Set gbuffer resources
	mpDeviceContext->OMSetRenderTargets(1, &this->mAmbientOcclusionBufferTexture->rtv, nullptr);
	mpDeviceContext->PSSetShaderResources(0, 1, &mAmbientOcclusionTexture->srv);
	gBlurParamatersDataPtr->blurHorizontal = true;
	this->mpBlurCB->UpdateBuffer((void*)gBlurParamatersDataPtr.get(), mpDeviceContext.Get());

	mpDeviceContext->Draw(4, 0);

	// IMPORTANT, need to do this nullptr hack otherwise it will complain about the resource still being bound as shader resource
	// (since we are using the same resources in the previous blurr pass
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	mpDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);

	mpDeviceContext->OMSetRenderTargets(1, &this->mAmbientOcclusionTexture->rtv, nullptr);
	mpDeviceContext->PSSetShaderResources(0, 1, &mAmbientOcclusionBufferTexture->srv);
	gBlurParamatersDataPtr->blurHorizontal = false;
	this->mpBlurCB->UpdateBuffer((void*)gBlurParamatersDataPtr.get(), mpDeviceContext.Get());

	mpDeviceContext->Draw(4, 0);
}


void Renderer::RenderSceneSSAOPass()
{
	// Get the fullscreen shaders
	VertexShader*const tFVS = mpShaderManager->GetVertexShader("Shaders\\VS_SSAO.hlsl");
	PixelShader*const tFPS = mpShaderManager->GetPixelShader("Shaders\\PS_SSAO.hlsl");


	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);

	// Set backbuffer as RT
	mpDeviceContext->OMSetRenderTargets(1, &this->mAmbientOcclusionTexture->rtv, nullptr);
	//mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);


	// Draw full screen quad
	//mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpPointWrapSampler);

	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);

	// Set gbuffer resources
	mpDeviceContext->PSSetShaderResources(0, 1, &gBuffer_albedoBuffer->srv);
	mpDeviceContext->PSSetShaderResources(1, 1, &gBuffer_positionBuffer->srv);
	mpDeviceContext->PSSetShaderResources(2, 1, &gBuffer_normalBuffer->srv);
	mpDeviceContext->PSSetShaderResources(3, 1, &randomValueTexture->srv);

	mpDeviceContext->Draw(4, 0);
}

void Renderer::RenderSceneLightingPass(std::vector<std::unique_ptr<IObject>>& aObjects)
{
	// Get the fullscreen shaders
	VertexShader*const tFVS = mpShaderManager->GetVertexShader("Shaders\\VS_DeferredLighting.hlsl");
	PixelShader*const tFPS = mpShaderManager->GetPixelShader("Shaders\\PS_DeferredLighting.hlsl");


	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);

	// Set backbuffer as RT
	mpDeviceContext->OMSetRenderTargets(1, &this->mBackBufferTexture->rtv, mBackBufferTexture->dsv);
	mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	
	// Draw full screen quad
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpPointWrapSampler);

	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);

	// Set gbuffer resources
	mpDeviceContext->PSSetShaderResources(0, 1, &gBuffer_albedoBuffer->srv);
	mpDeviceContext->PSSetShaderResources(1, 1, &gBuffer_positionBuffer->srv);
	mpDeviceContext->PSSetShaderResources(2, 1, &gBuffer_normalBuffer->srv);
	mpDeviceContext->PSSetShaderResources(3, 1, &mAmbientOcclusionTexture->srv);
	mpDeviceContext->PSSetShaderResources(4, 1, &mShadowDepthBuffer->srv);

	mpDeviceContext->Draw(4, 0);
}

void Renderer::BindStandardConstantBuffers()
{
	// view/Proj matrix cb at {0}
	ID3D11Buffer* tBuff = mpMatrixCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(0, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(0, 1, &tBuff);

	// Material info cb at {1}
	tBuff = mpMaterialCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(1, 1, &tBuff);

	// Light info for lighting cb at {2}
	tBuff = mpLightCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(2, 1, &tBuff);

	// light matrix for shadows cb at {3}
	tBuff = mpLightMatrixCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(3, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(3, 1, &tBuff);

	// Per object cb at {4}
	tBuff = mpPerObjectCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(4, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(4, 1, &tBuff);

	tBuff = mpBlurCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(5, 1, &tBuff);
}


void Renderer::RenderSceneDeferred(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera)
{
	//float color[4]{ clearColor[0], clearColor[1], clearColor[2], 1.0f };
	apCamera->UpdateViewMatrix();
	UpdateFrameConstantBuffers(aLights, aDirectionalLight, apCamera);

	BindStandardConstantBuffers();
	RenderSceneDepthPrePass(aObjects);
	RenderSceneGBufferFill(aObjects);
	RenderSceneSSAOPass();
	RenderBlurPass();
	RenderSceneLightingPass(aObjects);
}


void Renderer::RenderSceneForward(
std::vector<std::unique_ptr<IObject>>& aObjects, 
std::vector<std::unique_ptr<Light>>& aLights, 
d3dLightClass* const aDirectionalLight,
Camera* const apCamera, 
IObject* const aSkybox)
{
	float color[4]{ clearColor[0], clearColor[1], clearColor[2], 1.0f };

	auto frameConstantBufferTimerStart = std::chrono::high_resolution_clock::now();
	UpdateFrameConstantBuffers(aLights, aDirectionalLight, apCamera);

	// bind buffers
	BindStandardConstantBuffers();
	
	auto frameConstantBufferTimerEnd = std::chrono::high_resolution_clock::now();
	auto renderSceneDepthPrePassTimerStart = std::chrono::high_resolution_clock::now();
	RenderSceneDepthPrePass(aObjects);
	auto renderSceneDepthPrePassTimerEnd = std::chrono::high_resolution_clock::now();

	auto renderSceneWithShadowsTimerStart = std::chrono::high_resolution_clock::now();
	RenderSceneWithShadows(aObjects, aLights, aDirectionalLight, apCamera);
	auto renderSceneWithShadowsTimerEnd = std::chrono::high_resolution_clock::now();

	RenderSceneSkybox(aSkybox);
	
	// Render post processing quad
	RenderFullScreenQuad();

	static int frames = 0;
	++frames;


	// every 15 frames update
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

	VertexShader*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_depth.hlsl");
	//d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_depth.hlsl");

	//UpdateFrameConstantBuffers(aScene);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);

	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->inputLayout);

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
	VertexShader*const tFVS = mpShaderManager->GetVertexShader("Shaders\\fullScreenQuad_VS.hlsl");
	PixelShader*const tFPS = mpShaderManager->GetPixelShader("Shaders\\fullScreenQuad_PS.hlsl");


	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(1, &this->mBackBufferTexture->rtv, mBackBufferTexture->dsv);

	mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Draw full screen quad
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);
	mpDeviceContext->PSSetShaderResources(0, 1, &mPostProcColorBuffer->srv);
	mpDeviceContext->Draw(4, 0);
}

void Renderer::RenderMaterial(Material* const aMaterial)
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
	Material* const aMaterial = model->material;
	
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

float lerp(float v0, float v1, float t)
{
	return (1 - t) * v0 + t * v1;
}

bool Renderer::InitializeResources()
{	
	//mSceneRenderTexture = std::make_unique<d3dRenderTexture>();
	//mSceneRenderTexture->Initialize(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, SCREEN_NEAR, SCREEN_FAR);

	mBackBufferTexture = std::make_unique<Texture>();
	mPostProcColorBuffer = std::make_unique<Texture>();
	mPostProcDepthBuffer = std::make_unique<Texture>();
	mShadowDepthBuffer = std::make_unique<Texture>();

	gBuffer_positionBuffer = std::make_unique<Texture>();
	gBuffer_albedoBuffer = std::make_unique<Texture>();
	gBuffer_normalBuffer = std::make_unique<Texture>();
	gBuffer_specularBuffer = std::make_unique<Texture>();
	gBuffer_depthBuffer = std::make_unique<Texture>();
	randomValueTexture = std::make_unique<Texture>();

	mAmbientOcclusionTexture = std::make_unique<Texture>();
	mAmbientOcclusionBufferTexture = std::make_unique<Texture>();

	// Post proc color and depth buffer 
	mPostProcColorBuffer->texture =  CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	mPostProcColorBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), mPostProcColorBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mPostProcColorBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mPostProcColorBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	
	mPostProcDepthBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	mPostProcDepthBuffer->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), mPostProcDepthBuffer->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);

	// Backbuffer
	mBackBufferTexture->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	mBackBufferTexture->dsv =  CreateSimpleDepthstencilView(mpDevice.Get(), mBackBufferTexture->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);
	mBackBufferTexture->rtv = CreateRenderTargetViewFromSwapchain(mpDevice.Get(), mpSwapchain.Get());

	// Buffer for shadow mapping
	mShadowDepthBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), 8096, 8096, GetDepthResourceFormat(DXGI_FORMAT_D24_UNORM_S8_UINT), D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	mShadowDepthBuffer->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), mShadowDepthBuffer->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);
	mShadowDepthBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mShadowDepthBuffer->texture, GetDepthSRVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT));

	// Every buffer in the gbuffer needs a shader resource view and render target view, SRV for texture access, RTV to render 
	gBuffer_albedoBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	gBuffer_albedoBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), gBuffer_albedoBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	gBuffer_albedoBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), gBuffer_albedoBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);

	gBuffer_positionBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	gBuffer_positionBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), gBuffer_positionBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	gBuffer_positionBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), gBuffer_positionBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);

	gBuffer_normalBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	gBuffer_normalBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), gBuffer_normalBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	gBuffer_normalBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), gBuffer_normalBuffer->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);

	gBuffer_depthBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	gBuffer_depthBuffer->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), gBuffer_depthBuffer->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);

	// Our ambient occlusion texture is R32 format 
	mAmbientOcclusionTexture->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	mAmbientOcclusionTexture->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mAmbientOcclusionTexture->texture, DXGI_FORMAT_R32_FLOAT);
	mAmbientOcclusionTexture->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), mAmbientOcclusionTexture->texture, DXGI_FORMAT_R32_FLOAT);

	// Buffer texture used during the blur
	mAmbientOcclusionBufferTexture->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	mAmbientOcclusionBufferTexture->srv = CreateSimpleShaderResourceView(mpDevice.Get(), mAmbientOcclusionBufferTexture->texture, DXGI_FORMAT_R32_FLOAT);
	mAmbientOcclusionBufferTexture->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), mAmbientOcclusionBufferTexture->texture, DXGI_FORMAT_R32_FLOAT);

	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	
	
	// create offset positions to sample for our SSAO
	std::vector<VEC4f> randomValues;
	for (int i = 0; i < 16; ++i)
	{
		VEC4f value;
		// transform from range from 0 to 1  to -1 to 1
 		value.x = randomFloats(generator) *  2.0f - 1.0f; 
		value.y = randomFloats(generator) *  2.0f - 1.0f;
		value.z = 0.0f;
		value.w = 0.0f;
		randomValues.push_back(value);
	}

	D3D11_SUBRESOURCE_DATA srd;
	srd.pSysMem =(void*)&randomValues[0];
	// 4x4 pixel, so our pitch lines are 4
	srd.SysMemPitch = sizeof(VEC4f) * 4;
	srd.SysMemSlicePitch = 0;


	D3D11_TEXTURE2D_DESC texDesc;
	
	// Setup the render target texture description.
	// Set up the description of the depth buffer.
	texDesc.Width = 4;
	texDesc.Height = 4;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HRESULT hr = mpDevice->CreateTexture2D(&texDesc, &srd, &randomValueTexture->texture);

	randomValueTexture->srv = CreateSimpleShaderResourceView(mpDevice.Get(), randomValueTexture->texture, DXGI_FORMAT_R32G32B32A32_FLOAT);

	if (FAILED(hr))
	{
		std::cout << "Failed to create custom texture";
	}

	// we want x amount of kernels
	// needs to be in tangent space
	// Create hemisphere values in tangent space, 
	// x from -1 to 1
	// y from -1 to 1
	// z from 0 to 1 because this is a hemisphere, not full sphere
	// divide all values by 64

	memset(&gLightMatrixBufferDataPtr->kernelSamples, 0, sizeof(VEC3f) * (size_t)64);

	for (int i = 0; i < 64; ++i)
	{
		VEC3f value;
		value.x = randomFloats(generator) * 2.0f - 1.0f;
		value.y = randomFloats(generator) * 2.0f - 1.0f;
		value.z = randomFloats(generator);

		// normalize
		float length = sqrt(value.x*value.x + value.y*value.y + value.z*value.z);
		value.x /= length;
		value.y /= length;
		value.z /= length;
		
		float randValue = randomFloats(generator);
		value.x *= randValue;
		value.y *= randValue;
		value.z *= randValue;

		// 
		float scale = (float)i / 64.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		value.x *= scale;
		value.y *= scale;
		value.z *= scale;

		gLightMatrixBufferDataPtr->kernelSamples[i] = value;
	}
	return true;
}

#include "d3d11HelperFile.h"
bool  Renderer::InitializeDepthStencilView()
{
	mpDepthStencilState = CreateDepthStateDefault(mpDevice.Get());
	mDepthStencilStateLessEqual = CreateDepthStateLessEqual(mpDevice.Get());

	return true;
}


bool  Renderer::InitializeRasterstate()
{
	this->mRaster_backcull = CreateRSDefault(mpDevice.Get());
	this->mRaster_noCull = CreateRSNoCull(mpDevice.Get());

	return true;
}


bool Renderer::DestroyDirectX()
{
	mpShaderManager->ReleaseResources();

	ReleaseTexture(mBackBufferTexture.get());
	ReleaseTexture(mShadowDepthBuffer.get());

	ReleaseTexture(mPostProcColorBuffer.get());
	ReleaseTexture(mPostProcDepthBuffer.get());
	

	ReleaseTexture(gBuffer_positionBuffer.get());
	ReleaseTexture(gBuffer_albedoBuffer.get());
	ReleaseTexture(gBuffer_normalBuffer.get());
	ReleaseTexture(gBuffer_specularBuffer.get());
	ReleaseTexture(gBuffer_depthBuffer.get());

	ReleaseTexture(randomValueTexture.get());

	ReleaseTexture(mAmbientOcclusionTexture.get());
	ReleaseTexture(mAmbientOcclusionBufferTexture.get());

	mRaster_backcull->Release();
	mpDepthStencilState->Release();
	
	mRaster_noCull->Release();
	mDepthStencilStateLessEqual->Release();
	
	mpAnisotropicWrapSampler->Release();
	mpLinearClampSampler->Release();
	mpPointClampSampler->Release();
	mpLinearWrapSampler->Release();
	mpPointWrapSampler->Release();
	return true;
}


bool  Renderer::InitializeSamplerState()
{
	mpAnisotropicWrapSampler = CreateSamplerAnisotropicWrap(mpDevice.Get());
	mpPointClampSampler = CreateSamplerPointClamp(mpDevice.Get());
	mpLinearClampSampler = CreateSamplerLinearClamp(mpDevice.Get());
	mpLinearWrapSampler = CreateSamplerLinearWrap(mpDevice.Get());
	mpPointWrapSampler = CreateSamplerPointWrap(mpDevice.Get());
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

	if (!InitializeResources())
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
