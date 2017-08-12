#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#include "WindowsAndDXIncludes.h"


#include "d3dShaderManager.h"
#include "d3dConstantBuffer.h"
#include "d3dLightClass.h"

#include "IScene.h"
#include "d3d11HelperFile.h"
#include "GenericMathValueStructs.h"
#include "GPUProfiler.h"

class FrustumG;

class Renderer
{
public:
	Renderer();
	~Renderer();
	
	void Initialize(HWND aHwnd);

	 
	void RenderScene(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera, IObject* const aSkybox);
	//void Destroy();

	//void OnResize();
	
	Microsoft::WRL::ComPtr<IDXGISwapChain> mpSwapchain;
	Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;

	Microsoft::WRL::ComPtr<ID3D11Device1> mpDevice1;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> mpDeviceContext1;


	ID3D11Query* queryTestTimestampBegin;
	ID3D11Query* queryTestTimestampEnd;

	ID3D11Query* queryTestDisjoint;

	bool DestroyDirectX();

	GPUProfiler* tProfiler = nullptr;
private:
	bool InitializeDirectX();
	bool InitializeDXGI();
	bool InitializeDeviceAndContext();
	bool InitializeSwapchain();
	bool InitializeResources();
	bool InitializeViewportAndMatrices();

	void BindStandardConstantBuffers();

	void UpdateObjectConstantBuffers(IObject* const aObject);
	void UpdateShadowLightConstantBuffers(d3dLightClass* const aDirectionalLight);
	void UpdateFrameConstantBuffers(std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera);
	void UpdateGenericConstantBuffer(float aScreenWidth, float aScreenHeight, float nearPlaneDistance, float farPlaneDistance);


	void CreateConstantBuffers();

	// Render functions

	void CullObjects(std::vector<std::unique_ptr<IObject>>& aObjectsToCull, const FrustumG* const aCullFrustum);
	void RenderObject(IObject* const aObject);
	void RenderMaterial(Material* const aMaterial);

	void RenderFullScreenQuad();
	
	void RenderSceneForward(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<IObject*>& aCulledObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera, IObject* const aSkybox);
	void RenderSceneDeferred(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<IObject*>& aCulledObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera, IObject* const aSkyboxObject);

	void RenderSceneSSAOPass();
	void RenderBlurPass();
	void RenderSceneGBufferFill(std::vector<IObject*>& aObjects);
	void RenderSceneLightingPass(std::vector<std::unique_ptr<IObject>>& aObjects, IObject* const aSkyboxObject);
	void RenderSceneSkybox(IObject* const aObject);

	void RenderSceneDepthPrePass(std::vector<std::unique_ptr<IObject>>& aObjects);
	void RenderSceneWithShadows(std::vector<IObject*>& aObjects,
		std::vector<std::unique_ptr<Light>>& aLights,
		d3dLightClass* const aDirectionalLight,
		Camera* const apCamera);
	
	
	void RenderBuffers(ID3D11DeviceContext* const apDeviceContext, Model* const aModel);

	int gVideoCardMemoryAmount;
	char gVideoCardDescription[128];

	int gnumerator, gdenominator;

	HWND windowHandle;

	D3D11_VIEWPORT mViewport;

	std::unique_ptr<FrustumG> mMainCamCullFrustum;

	// IDXGI stuff
	Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;

	ID3D11DepthStencilState* mpDepthStencilState;
	ID3D11RasterizerState* mRaster_backcull;
	
	ID3D11RasterizerState* mRaster_cullNone;

	ID3D11RasterizerState* mRaster_noCull;
	ID3D11DepthStencilState* mDepthStencilStateLessEqual;

	ID3D11DepthStencilState* mDepthStencilDeferred;

	// Samplers
	ID3D11SamplerState* mpAnisotropicWrapSampler;
	ID3D11SamplerState* mpLinearClampSampler;
	ID3D11SamplerState* mpLinearWrapSampler;

	ID3D11SamplerState* mpPointClampSampler;
	ID3D11SamplerState* mpPointWrapSampler;

	// Constant buffers 
	std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
	std::unique_ptr<d3dConstantBuffer> mpLightCB;
	std::unique_ptr<d3dConstantBuffer> mpLightMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpPerObjectCB;
	std::unique_ptr<d3dConstantBuffer> mpBlurCB;
	std::unique_ptr<d3dConstantBuffer> mpGenericAttributesBufferCB;


	// Shader manager
	std::unique_ptr<d3dShaderManager> mpShaderManager;

	std::unique_ptr<Texture> mBackBufferTexture;

	std::unique_ptr<Texture> mAmbientOcclusionTexture;
	std::unique_ptr<Texture> mAmbientOcclusionBufferTexture;

	std::unique_ptr<Texture> mPostProcColorBuffer;
	std::unique_ptr<Texture> mPostProcDepthBuffer;

	std::unique_ptr<Texture> gBuffer_positionBuffer;
	std::unique_ptr<Texture> gBuffer_albedoBuffer;
	std::unique_ptr<Texture> gBuffer_normalBuffer;
	std::unique_ptr<Texture> gBuffer_specularBuffer;
	std::unique_ptr<Texture> gBuffer_depthBuffer;
	std::unique_ptr<Texture> randomValueTexture;

	std::unique_ptr<ShadowMap> shadowMap01;


	uint32_t sphereID;
};

