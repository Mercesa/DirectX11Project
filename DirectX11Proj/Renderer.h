#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "WindowsAndDXIncludes.h"


#include "d3dShaderManager.h"
#include "d3dConstantBuffer.h"
#include "d3dLightClass.h"

#include "IScene.h"
#include "d3d11HelperFile.h"
#include "GenericMathValueStructs.h"

class d3dRenderTexture;
class d3dRenderDepthTexture;
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

	float clearColor[3] = { 0.0f, 0.0f, 0.0f };

		bool DestroyDirectX();

private:
	bool InitializeDirectX();
	bool InitializeDXGI();
	bool InitializeDeviceAndContext();
	bool InitializeSwapchain();
	bool InitializeResources();
	bool InitializeDepthStencilView();
	bool InitializeRasterstate();
	bool InitializeSamplerState();
	bool InitializeViewportAndMatrices();

	void BindStandardConstantBuffers();

	void UpdateObjectConstantBuffers(IObject* const aObject);
	void UpdateShadowLightConstantBuffers(d3dLightClass* const aDirectionalLight);
	void UpdateFrameConstantBuffers(std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera);
	
	void CreateConstantBuffers();

	// Render functions
	void RenderObject(IObject* const aObject);
	void RenderMaterial(Material* const aMaterial);

	void RenderFullScreenQuad();
	
	void RenderSceneForward(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera, IObject* const aSkybox);
	void RenderSceneDeferred(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera);

	void RenderSceneSSAOPass();
	void RenderBlurPass();
	void RenderSceneGBufferFill(std::vector<std::unique_ptr<IObject>>& aObjects);
	void RenderSceneLightingPass(std::vector<std::unique_ptr<IObject>>& aObjects);
	void RenderSceneSkybox(IObject* const aObject);

	void RenderSceneDepthPrePass(std::vector<std::unique_ptr<IObject>>& aObjects);
	void RenderSceneWithShadows(std::vector<std::unique_ptr<IObject>>& aObjects,
		std::vector<std::unique_ptr<Light>>& aLights,
		d3dLightClass* const aDirectionalLight,
		Camera* const apCamera);
	
	
	void RenderBuffers(ID3D11DeviceContext* const apDeviceContext, Model* const aModel);

	const float SCREEN_FAR = 1000.0f;
	const float SCREEN_NEAR = 2.0f;

	int gVideoCardMemoryAmount;
	char gVideoCardDescription[128];

	int gnumerator, gdenominator;

	HWND windowHandle;

	D3D11_VIEWPORT mViewport;
	D3D11_VIEWPORT mShadowLightViewport;

	
	std::unique_ptr<FrustumG> mMainCamCullFrustum;
	Model* frustumModel;


	// IDXGI stuff
	Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;

	ID3D11DepthStencilState* mpDepthStencilState;
	ID3D11RasterizerState* mRaster_backcull;
	

	ID3D11RasterizerState* mRaster_noCull;
	ID3D11DepthStencilState* mDepthStencilStateLessEqual;

	//Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRaster_frontcull; not used yet

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

	// Shader manager
	std::unique_ptr<d3dShaderManager> mpShaderManager;

	std::unique_ptr<Texture> mBackBufferTexture;
	std::unique_ptr<Texture> mShadowDepthBuffer;

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
};

