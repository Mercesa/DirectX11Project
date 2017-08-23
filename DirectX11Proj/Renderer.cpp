#include <chrono>

#include "Renderer.h"

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
#include "FrustumG.h"
#include "d3d11HelperFile.h"

#include <random>

#define USE_MOTIONBLURRECONSTRUCTION 

using namespace DirectX;

static bool renderForward = true;
static bool firstFrame = true;
static bool UpdateCullFrustum = true;
static float objectRenderingTime = 0.0f;

std::vector<IObject*> mCulledObjects;

// Data ptrs for constant buffer data
static std::unique_ptr<cbLights> gLightBufferDataPtr = std::make_unique<cbLights>();
static std::unique_ptr<cbMaterial> gMaterialBufferDataPtr = std::make_unique<cbMaterial>();
static std::unique_ptr<cbMatrixBuffer> gMatrixBufferDataPtr = std::make_unique<cbMatrixBuffer>();
static std::unique_ptr<cbLightMatrix> gLightMatrixBufferDataPtr = std::make_unique<cbLightMatrix>();
static std::unique_ptr<cbPerObject> gPerObjectMatrixBufferDataPtr = std::make_unique<cbPerObject>();
static std::unique_ptr<cbBlurParameters> gBlurParamatersDataPtr = std::make_unique<cbBlurParameters>();
static std::unique_ptr<cbGenericAttributesBuffer> gGenericAttributesDataPtr = std::make_unique<cbGenericAttributesBuffer>();


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

void Renderer::CullObjects(std::vector<std::unique_ptr<IObject>>& aObjectsToCull, const FrustumG* const aCullFrustum)
{
	size_t aLoopSize = aObjectsToCull.size();

	for (int i = 0; i < aLoopSize; ++i)
	{
		IObject* const tObj = aObjectsToCull[i].get();

		if (aCullFrustum->sphereInFrustum(tObj->mSpherePosition) == 1)
		{
			mCulledObjects.push_back(tObj);
		}
	}
}

void Renderer::RenderTexturesToScreen(Texture* const aTextures)
{
	D3D11_VIEWPORT viewport1;

	viewport1.Height = 320;
	viewport1.Width = 320;
	viewport1.MaxDepth = 1.0f;
	viewport1.MinDepth = 0.0f;
	viewport1.TopLeftX = 0.0f;
	viewport1.TopLeftY = 0.0f;

	// Get the fullscreen shaders
	VertexShader*const tFVS = mpShaderManager->GetVertexShader("Shaders\\VS_TextureToScreen.hlsl");
	PixelShader*const tFPS = mpShaderManager->GetPixelShader("Shaders\\PS_TextureToScreen.hlsl");


	mpDeviceContext->RSSetViewports(1, &viewport1);
	mpDeviceContext->RSSetState(mRaster_cullNone);

	mpDeviceContext->OMSetRenderTargets(1, &this->mBackBufferTexture->rtv, mBackBufferTexture->dsv);

	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);

	mpDeviceContext->PSSetSamplers(0, 1, &mpLinearWrapSampler);
	mpDeviceContext->IASetInputLayout(tFVS->inputLayout);

	// Set gbuffer resources
	mpDeviceContext->PSSetShaderResources(0, 1, &aTextures->srv);

	mpDeviceContext->Draw(4, 0);
}

void Renderer::RenderScene(
	std::vector<std::unique_ptr<IObject>>& aObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	LightData* const aDirectionalLight,
	CameraData apCamera,
	IObject* const aSkybox,
	const FrameData* const aFrameData)

{
	ImGui::MenuItem("RenderForward", NULL, &renderForward);
	ImGui::MenuItem("UpdateCullFrustum", NULL, &UpdateCullFrustum);

	// Update cull frustum with current position


	if (firstFrame)
	{
		mCulledObjects.reserve(2048);
		firstFrame = false;
	}

	mCulledObjects.clear();

	CullObjects(aObjects, apCamera.frustumPtr);

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "ConstantBuffersTime");

	BindStandardConstantBuffers();
	UpdateFrameConstantBuffers(aLights, aDirectionalLight, apCamera);
	UpdateGenericConstantBuffer(GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, apCamera.nearZ, apCamera.farZ, aFrameData);

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "ConstantBuffersTime");


	if (renderForward)
	{
		RenderSceneForward(aObjects, mCulledObjects, aLights, aDirectionalLight, apCamera, aSkybox);
	}

	else
	{
		RenderSceneDeferred(aObjects, mCulledObjects, aLights, aDirectionalLight, apCamera, aSkybox);
	}
}


void Renderer::RenderSceneDeferred(
	std::vector<std::unique_ptr<IObject>>& aObjects, 
	std::vector<IObject*>& aCulledObjects, 
	std::vector<std::unique_ptr<Light>>& aLights, 
	LightData* const aDirectionalLight, 
	CameraData const apCamera,
	IObject* const aSkyboxObject)
{
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "DepthPre-pass");
	RenderSceneDepthPrePass(aObjects);
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "DepthPre-pass");

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "GbufferFill");
	RenderSceneGBufferFill(aCulledObjects);
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "GbufferFill");

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SSAOPass");
	RenderSceneSSAOPass();
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SSAOPass");

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SSAOBlur");
	RenderBlurPass();
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SSAOBlur");

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "MotionBlurVelocityPass");
	RenderSceneVelocityPass(aCulledObjects);
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "MotionBlurVelocityPass");

	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "LightingPass");
	RenderSceneLightingPass(aObjects, aSkyboxObject);
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "LightingPass");

	//RenderTexturesToScreen(reconstruction_VelocityBuffer.get());
}


void Renderer::RenderSceneForward(
	std::vector<std::unique_ptr<IObject>>& aObjects,
	std::vector<IObject*>& aCulledObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	LightData* const aDirectionalLight,
	CameraData apCamera,
	IObject* const aSkybox)
{
	// 
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SceneDepthPrePass");
	auto renderSceneDepthPrePassTimerStart = std::chrono::high_resolution_clock::now();
	
	RenderSceneDepthPrePass(aObjects);
	
	auto renderSceneDepthPrePassTimerEnd = std::chrono::high_resolution_clock::now();
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SceneDepthPrePass");


	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SceneShadows");
	auto renderSceneWithShadowsTimerStart = std::chrono::high_resolution_clock::now();
	
	RenderSceneWithShadows(aCulledObjects, aLights, aDirectionalLight, apCamera);
	
	auto renderSceneWithShadowsTimerEnd = std::chrono::high_resolution_clock::now();
	tProfiler->SetStamp(mpDeviceContext.Get(), mpDevice.Get(), "SceneShadows");


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
	static int drawCalls = 0; 
	if (frames % 15 == 0)
	{
		//fCbtiming = std::chrono::duration_cast<std::chrono::microseconds>(frameConstantBufferTimerEnd - frameConstantBufferTimerStart).count() / 1000.0f;
		depthPrePassTiming = std::chrono::duration_cast<std::chrono::microseconds>(renderSceneDepthPrePassTimerEnd - renderSceneDepthPrePassTimerStart).count() / 1000.0f;
		shadowSceneTiming = std::chrono::duration_cast<std::chrono::microseconds>(renderSceneWithShadowsTimerEnd - renderSceneWithShadowsTimerStart).count() / 1000.0f;
		shadowSceneObjectTiming = objectRenderingTime;
	}

	drawCalls = static_cast<int>(mCulledObjects.size());

	ImGui::Text("Updating frame constant %.5f ms/frame", fCbtiming);
	ImGui::Text("Render scene depth pre-pass %.5f ms/frame", depthPrePassTiming);
	ImGui::Text("Render scene with shadows %.5f ms/frame", shadowSceneTiming);
	ImGui::Text("Render shadow scene objects %.5f ms/frame", shadowSceneObjectTiming);
	ImGui::Text("Drawcalls: %i ", drawCalls);
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
	mpGenericAttributesBufferCB = std::make_unique<d3dConstantBuffer>(sizeof(cbGenericAttributesBuffer), nullptr, mpDevice.Get());

	LOG(INFO) << "Constant buffers created";
}


void Renderer::UpdateGenericConstantBuffer(float aScreenWidth, float aScreenHeight, float nearPlaneDistance, float farPlaneDistance, const FrameData* const aFrameData)
{
	gGenericAttributesDataPtr->screenWidth = aScreenWidth;
	gGenericAttributesDataPtr->screenHeight = aScreenHeight;
	gGenericAttributesDataPtr->nearPlaneDistance = nearPlaneDistance;
	gGenericAttributesDataPtr->farPlaneDistance = farPlaneDistance;
	
	gGenericAttributesDataPtr->deltaTime = aFrameData->deltaTime;
	gGenericAttributesDataPtr->totalApplicationTime = aFrameData->totalTime;
	gGenericAttributesDataPtr->framerate = aFrameData->framerate;

	mpGenericAttributesBufferCB->UpdateBuffer((void*)gGenericAttributesDataPtr.get(), mpDeviceContext.Get());
}


void Renderer::UpdateFrameConstantBuffers(std::vector <std::unique_ptr<Light>>& apLights, LightData* const aDirectionalLight, CameraData apCamera)
{
	// Update light constant buffers
	gLightBufferDataPtr->amountOfLights = static_cast<int>(apLights.size());

	for (int i = 0; i < apLights.size(); ++i)
	{
		gLightBufferDataPtr->arr[i].position = apLights[i]->position;
		gLightBufferDataPtr->arr[i].diffuseColor = apLights[i]->diffuseColor;
	}

	gLightBufferDataPtr->directionalLight.diffuseColor = aDirectionalLight->diffuseCol;
	gLightBufferDataPtr->directionalLight.specularColor = aDirectionalLight->specularCol;
	gLightBufferDataPtr->directionalLight.position = aDirectionalLight->dirVector;

	
	mpLightCB->UpdateBuffer((void*)gLightBufferDataPtr.get(), mpDeviceContext.Get());
	
	// Copy the matrices into the constant buffer
	// Initialize with proj view matrix from last frame to set the previous matrix 
	gMatrixBufferDataPtr->prevProjViewMatrix = gMatrixBufferDataPtr->projViewMatrix;
	
	
	gMatrixBufferDataPtr->view = glm::transpose(apCamera.view);
	gMatrixBufferDataPtr->projection = glm::transpose(apCamera.proj);
	gMatrixBufferDataPtr->viewMatrixInversed = glm::transpose(glm::inverse(apCamera.view));
	gMatrixBufferDataPtr->projectionMatrixInverse = glm::transpose(glm::inverse(apCamera.proj));
	gMatrixBufferDataPtr->projViewMatrix = gMatrixBufferDataPtr->view*gMatrixBufferDataPtr->projection;

	gMatrixBufferDataPtr->gEyePosX = apCamera.position.x;
	gMatrixBufferDataPtr->gEyePosY = apCamera.position.y;
	gMatrixBufferDataPtr->gEyePosZ = apCamera.position.z;


	mpMatrixCB->UpdateBuffer((void*)gMatrixBufferDataPtr.get(), mpDeviceContext.Get());


	UpdateShadowLightConstantBuffers(aDirectionalLight);
}


void Renderer::UpdateShadowLightConstantBuffers(LightData* const aDirectionalLight)
{
	gLightMatrixBufferDataPtr->lightProjectionMatrix = glm::transpose(aDirectionalLight->proj);
	gLightMatrixBufferDataPtr->lightViewMatrix = glm::transpose(aDirectionalLight->view);;
	gLightMatrixBufferDataPtr->lightProjectionViewMatrix = gLightMatrixBufferDataPtr->lightViewMatrix * gLightMatrixBufferDataPtr->lightProjectionMatrix;


	gLightMatrixBufferDataPtr->shadowMapWidth = shadowMap01->width;
	gLightMatrixBufferDataPtr->shadowMapheight = shadowMap01->height;
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

	// Update buffer with world matrix
	gPerObjectMatrixBufferDataPtr->worldMatrix = aObject->mWorldMatrix;
	gPerObjectMatrixBufferDataPtr->prevWorldMatrix = aObject->mPrevWorldMatrix;

	mpPerObjectCB->UpdateBuffer((void*)gPerObjectMatrixBufferDataPtr.get(), mpDeviceContext.Get());
}


void Renderer::RenderSceneWithShadows(std::vector<IObject*>& aObjects,
	std::vector<std::unique_ptr<Light>>& aLights,
	LightData* const aDirectionalLight,
	CameraData apCamera)
{
	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);


	mpDeviceContext->ClearDepthStencilView(this->mPostProcDepthBuffer->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(1, &mPostProcColorBuffer->rtv, this->mPostProcDepthBuffer->dsv);


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

	ID3D11ShaderResourceView* aView = shadowMap01->resource->srv;
	mpDeviceContext->PSSetShaderResources(3, 1, &aView);


	auto renderSceneObjectsStart = std::chrono::high_resolution_clock::now();
	
	// Render objects
	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i]);
		RenderObject(aObjects[i]);
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


void Renderer::RenderSceneGBufferFill(std::vector<IObject*>& aObjects)
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
		UpdateObjectConstantBuffers(aObjects[i]);
		RenderObject(aObjects[i]);
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

	//mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);

	// Set backbuffer as RT
	mpDeviceContext->OMSetRenderTargets(1, &this->mAmbientOcclusionTexture->rtv, this->gBuffer_depthBuffer->dsv);
	//mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);


	// Draw full screen quad
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpPointWrapSampler);

	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);

	// Set gbuffer resources
	mpDeviceContext->PSSetShaderResources(1, 1, &gBuffer_positionBuffer->srv);
	mpDeviceContext->PSSetShaderResources(2, 1, &gBuffer_normalBuffer->srv);
	mpDeviceContext->PSSetShaderResources(3, 1, &randomValueTexture->srv);

	mpDeviceContext->Draw(4, 0);
}


void Renderer::RenderSceneLightingPass(std::vector<std::unique_ptr<IObject>>& aObjects, IObject* const aSkyboxObject)
{
	// Get the fullscreen shaders
	VertexShader*const tFVS = mpShaderManager->GetVertexShader("Shaders\\VS_DeferredLighting.hlsl");
	PixelShader* tFPS = nullptr;

#ifdef USE_MOTIONBLURRECONSTRUCTION
	tFPS = mpShaderManager->GetPixelShader("Shaders\\PS_DeferredLightingReconstruction.hlsl");
#else
	tFPS = mpShaderManager->GetPixelShader("Shaders\\PS_DeferredLighting.hlsl");
#endif


	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_cullNone);

	mpDeviceContext->OMSetDepthStencilState(mDepthStencilDeferred, 1);

	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	// Set backbuffer as RT
	mpDeviceContext->ClearRenderTargetView(this->mBackBufferTexture->rtv, color);
	mpDeviceContext->OMSetRenderTargets(1, &this->mBackBufferTexture->rtv, mBackBufferTexture->dsv);
	mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	
	// Draw full screen quad
	mpDeviceContext->PSSetSamplers(0, 1, &mpPointClampSampler);
	mpDeviceContext->PSSetSamplers(1, 1, &mpPointWrapSampler);

	mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mpDeviceContext->VSSetShader(tFVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tFPS->shader, NULL, 0);

	mpDeviceContext->IASetInputLayout(tFVS->inputLayout);

	// Set gbuffer resources
	mpDeviceContext->PSSetShaderResources(0, 1, &gBuffer_albedoBuffer->srv);
	mpDeviceContext->PSSetShaderResources(1, 1, &gBuffer_positionBuffer->srv);
	mpDeviceContext->PSSetShaderResources(2, 1, &gBuffer_normalBuffer->srv);
	mpDeviceContext->PSSetShaderResources(3, 1, &mAmbientOcclusionTexture->srv);
	mpDeviceContext->PSSetShaderResources(4, 1, &shadowMap01->resource->srv);
	

	Texture* velocityTextureToUse = nullptr;
#ifdef USE_MOTIONBLURRECONSTRUCTION
	velocityTextureToUse = reconstruction_VelocityBuffer.get();
#else
	velocityTextureToUse = velocityTexture.get;
#endif
	mpDeviceContext->PSSetShaderResources(5, 1, &velocityTextureToUse->srv);




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

	tBuff = mpGenericAttributesBufferCB->GetBuffer();
	mpDeviceContext->VSSetConstantBuffers(6, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(6, 1, &tBuff);
}


// Render the scene to a depth map texture
void Renderer::RenderSceneDepthPrePass(std::vector<std::unique_ptr<IObject>>& aObjects)
{
	mpDeviceContext->RSSetViewports(1, &shadowMap01->viewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->ClearDepthStencilView(this->shadowMap01->resource->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(0, nullptr, shadowMap01->resource->dsv);

	VertexShader*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_depth.hlsl");
	//d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_depth.hlsl");

	//UpdateFrameConstantBuffers(aScene);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);

	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->inputLayout);

	for (int i = 0; i < aObjects.size(); ++i)
	{
		if (aObjects[i]->GetCastShadow())
		{
			UpdateObjectConstantBuffers(aObjects[i].get());
			RenderObject(aObjects[i].get());
		}
	}
}


void Renderer::RenderSceneVelocityPass(std::vector<IObject*>& aObjects)
{

	VertexShader* tVS = nullptr; 
	PixelShader* tPS = nullptr; 
	Texture* textureRT = nullptr;

	// Standard shader
	tVS = mpShaderManager->GetVertexShader("Shaders\\VS_VelocityBuffer.hlsl");
	
	// Use a different shader when using motion blur reconstruction
#ifdef USE_MOTIONBLURRECONSTRUCTION
	tPS = mpShaderManager->GetPixelShader("Shaders\\PS_VelocityBufferReconstruction.hlsl");
	textureRT = reconstruction_VelocityBuffer.get();
#else
	tPS = mpShaderManager->GetPixelShader("Shaders\\PS_VelocityBuffer.hlsl");
	textureRT = velocityTexture.get();
#endif

	mpDeviceContext->RSSetViewports(1, &mViewport);
	mpDeviceContext->RSSetState(mRaster_backcull);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	mpDeviceContext->OMSetRenderTargets(1, &reconstruction_VelocityBuffer->rtv, this->mBackBufferTexture->dsv);
	
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	mpDeviceContext->ClearRenderTargetView(reconstruction_VelocityBuffer->rtv, color);
	mpDeviceContext->ClearDepthStencilView(mBackBufferTexture->dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);
	mpDeviceContext->PSSetShader(tPS->shader, NULL, 0);

	// Set the sampler state in the pixel shader.
	mpDeviceContext->IASetInputLayout(tVS->inputLayout);

	for (int i = 0; i < aObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aObjects[i]);
		RenderObject(aObjects[i]);
	}


	// Tilemax and neighbour max for our reconstruction algorithm
#ifdef USE_MOTIONBLURRECONSTRUCTION

	ComputeShader* const tCS = mpShaderManager->GetComputeShader("Shaders\\CS_ReconstructionVelocityTileMax.hlsl");

	mpDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	mpDeviceContext->CSSetShader(tCS->shader, NULL, 0);
	mpDeviceContext->CSSetShaderResources(0, 1, &reconstruction_VelocityBuffer->srv);
	mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &reconstruction_TileMaxBuffer->uav, 0);

	mpDeviceContext->Dispatch(GraphicsSettings::gCurrentScreenWidth / 20, GraphicsSettings::gCurrentScreenHeight / 20, 1);

	//// Create viewport for reconstruction textures
	//D3D11_VIEWPORT reconstructionViewport;
	//
	//reconstructionViewport.MinDepth = 0.0f;
	//reconstructionViewport.MaxDepth = 1.0f;
	//reconstructionViewport.TopLeftX = 0.0f;
	//reconstructionViewport.TopLeftY = 0.0f;
	//reconstructionViewport.Width = GraphicsSettings::gCurrentScreenWidth;
	//reconstructionViewport.Height = GraphicsSettings::gCurrentScreenHeight;
	//mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState, 1);
	//
	//mpDeviceContext->RSSetViewports(1, &reconstructionViewport);
	//// To ensure the velocity buffer texture we have set 
	////mpDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	//
	//tVS = mpShaderManager->GetVertexShader("Shaders\\fullScreenQuad_VS.hlsl");
	//tPS = mpShaderManager->GetPixelShader("Shaders\\PS_VelocityBufferTileMax.hlsl");
	//mpDeviceContext->OMSetRenderTargets(1, &this->reconstruction_TileMaxBuffer->rtv, reconstruction_DepthBuffer->dsv);
	//mpDeviceContext->ClearDepthStencilView(reconstruction_DepthBuffer->dsv, D3D11_CLEAR_DEPTH, 0, 0);
	//
	//mpDeviceContext->VSSetShader(tVS->shader, NULL, 0);
	//mpDeviceContext->PSSetShader(tPS->shader, NULL, 0);
	//
	//mpDeviceContext->PSSetShaderResources(0, 1, &reconstruction_VelocityBuffer->srv);
	//
	//mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//mpDeviceContext->Draw(4, 0);
#endif
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
	Material* const aMaterial = aObject->mpMaterial;
	
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
	// Generic viewport initialization
	mViewport.Width = (float)GraphicsSettings::gCurrentScreenWidth;
	mViewport.Height = (float)GraphicsSettings::gCurrentScreenHeight;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;


	// Create sampler states
	mpAnisotropicWrapSampler = CreateSamplerAnisotropicWrap(mpDevice.Get());
	mpPointClampSampler = CreateSamplerPointClamp(mpDevice.Get());
	mpLinearClampSampler = CreateSamplerLinearClamp(mpDevice.Get());
	mpLinearWrapSampler = CreateSamplerLinearWrap(mpDevice.Get());
	mpPointWrapSampler = CreateSamplerPointWrap(mpDevice.Get());

	// Create depth stencil states
	mpDepthStencilState = CreateDepthStateDefault(mpDevice.Get());
	mDepthStencilStateLessEqual = CreateDepthStateLessEqual(mpDevice.Get());
	mDepthStencilDeferred = CreateDepthStateDeferred(mpDevice.Get());

	// Create raster states
	this->mRaster_backcull = CreateRSDefault(mpDevice.Get());
	this->mRaster_noCull = CreateRSNoCull(mpDevice.Get());
	mRaster_cullNone = CreateRSCullNone(mpDevice.Get());
	// Create textures
	mBackBufferTexture = std::make_unique<Texture>();
	mPostProcColorBuffer = std::make_unique<Texture>();
	mPostProcDepthBuffer = std::make_unique<Texture>();

	gBuffer_positionBuffer = std::make_unique<Texture>();
	gBuffer_albedoBuffer = std::make_unique<Texture>();
	gBuffer_normalBuffer = std::make_unique<Texture>();
	gBuffer_specularBuffer = std::make_unique<Texture>();
	gBuffer_depthBuffer = std::make_unique<Texture>();
	randomValueTexture = std::make_unique<Texture>();

	mAmbientOcclusionTexture = std::make_unique<Texture>();
	mAmbientOcclusionBufferTexture = std::make_unique<Texture>();
	
	velocityTexture = std::make_unique<Texture>();
	
	// Resources for motion blur deconstructing technique
	reconstruction_VelocityBuffer = std::make_unique<Texture>();
	reconstruction_TileMaxBuffer = std::make_unique<Texture>();
	reconstruction_DepthBuffer = std::make_unique<Texture>();

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


	// Generate shadow map 
	shadowMap01 = std::make_unique<ShadowMap>();
	shadowMap01->resource = new Texture();

	shadowMap01->viewport.Width = 8096.0f;
	shadowMap01->viewport.Height = 8096.0f;
	shadowMap01->viewport.MinDepth = 0.0f;
	shadowMap01->viewport.MaxDepth = 1.0f;
	shadowMap01->viewport.TopLeftX = 0.0f;
	shadowMap01->viewport.TopLeftY = 0.0f;

	shadowMap01->width = shadowMap01->viewport.Width;
	shadowMap01->height = shadowMap01->viewport.Height;


	// Buffer for shadow mapping
	shadowMap01->resource->texture = CreateSimpleTexture2D(mpDevice.Get(), shadowMap01->width, shadowMap01->height, GetDepthResourceFormat(DXGI_FORMAT_D24_UNORM_S8_UINT), D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	shadowMap01->resource->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), shadowMap01->resource->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);
	shadowMap01->resource->srv = CreateSimpleShaderResourceView(mpDevice.Get(), shadowMap01->resource->texture, GetDepthSRVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT));

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

	// Texture used to record velocity values into buffer
	velocityTexture->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R32G32_FLOAT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	velocityTexture->srv = CreateSimpleShaderResourceView(mpDevice.Get(), velocityTexture->texture, DXGI_FORMAT_R32G32_FLOAT);
	velocityTexture->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), velocityTexture->texture, DXGI_FORMAT_R32G32_FLOAT);


	// BUFFERS FOR RECONSTRUCTION TECHNIQUE
	// Texture used to record velocity values into buffer
	reconstruction_VelocityBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, DXGI_FORMAT_R8G8_SINT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	reconstruction_VelocityBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), reconstruction_VelocityBuffer->texture, DXGI_FORMAT_R8G8_SINT);
	reconstruction_VelocityBuffer->rtv = CreateSimpleRenderTargetView(mpDevice.Get(), reconstruction_VelocityBuffer->texture, DXGI_FORMAT_R8G8_SINT);

	reconstruction_TileMaxBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth / 20, GraphicsSettings::gCurrentScreenHeight / 20, DXGI_FORMAT_R8G8_SINT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
	reconstruction_TileMaxBuffer->srv = CreateSimpleShaderResourceView(mpDevice.Get(), reconstruction_TileMaxBuffer->texture, DXGI_FORMAT_R8G8_SINT);
	reconstruction_TileMaxBuffer->uav = CreateSimpleUnorderedAccessView(mpDevice.Get(), reconstruction_TileMaxBuffer->texture, DXGI_FORMAT_R8G8_SINT);

	reconstruction_DepthBuffer->texture = CreateSimpleTexture2D(mpDevice.Get(), GraphicsSettings::gCurrentScreenWidth / 20, GraphicsSettings::gCurrentScreenHeight / 20, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	reconstruction_DepthBuffer->dsv = CreateSimpleDepthstencilView(mpDevice.Get(), reconstruction_DepthBuffer->texture, DXGI_FORMAT_D24_UNORM_S8_UINT);
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

	for (int i = 0; i < 32; ++i)
	{
		VEC3f value;
		// Create random rotation vectors
		value.x = randomFloats(generator) * 2.0f - 1.0f;
		value.y = randomFloats(generator) * 2.0f - 1.0f;
		value.z = randomFloats(generator);

		// normalize
		float length = sqrt(value.x*value.x + value.y*value.y + value.z*value.z);
		value.x /= length;
		value.y /= length;
		value.z /= length;
		
		// Give it a random length
		float randValue = randomFloats(generator);
		value.x *= randValue;
		value.y *= randValue;
		value.z *= randValue;

		// Samples close to center should be scaled more
		float scale = (float)i / 32.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		value.x *= scale;
		value.y *= scale;
		value.z *= scale;

		gLightMatrixBufferDataPtr->kernelSamples[i] = value;
	}


	

	return true;
}

bool Renderer::DestroyDirectX()
{
	mpShaderManager->ReleaseResources();

	ReleaseTexture(mBackBufferTexture.get());
	ReleaseTexture(shadowMap01->resource);
	delete shadowMap01->resource;

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

	ReleaseTexture(velocityTexture.get());

	ReleaseTexture(reconstruction_TileMaxBuffer.get());
	ReleaseTexture(reconstruction_VelocityBuffer.get());
	ReleaseTexture(reconstruction_DepthBuffer.get());

	mRaster_backcull->Release();
	mpDepthStencilState->Release();
	
	mRaster_noCull->Release();
	mDepthStencilStateLessEqual->Release();
	mDepthStencilDeferred->Release();

	mRaster_cullNone->Release();

	mpAnisotropicWrapSampler->Release();
	mpLinearClampSampler->Release();
	mpPointClampSampler->Release();
	mpLinearWrapSampler->Release();
	mpPointWrapSampler->Release();
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
		LOG(INFO) << "Failed to initialize resources";
		return false;
	}
	LOG(INFO) << "Successfully initialized resources";


	LOG(INFO) << "Successfully initialized DirectX!";

	return true;
}