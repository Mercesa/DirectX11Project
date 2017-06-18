#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <windowsx.h>
#include <wrl.h>

#include <fcntl.h>
#include <d3d11.h>	
#include <directxmath.h>
#include "d3dShaderManager.h"
#include "d3dConstantBuffer.h"
#include "d3dLightClass.h"

#include "IScene.h"

class d3dRenderTexture;
class d3dRenderDepthTexture;

class Renderer
{
public:
	Renderer();
	~Renderer();
	
	void Initialize(HWND aHwnd);

	 
	void RenderScene(std::vector<std::unique_ptr<IObject>>& aObjects, std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera);
	//void Destroy();

	//void OnResize();
	
	Microsoft::WRL::ComPtr<IDXGISwapChain> mpSwapchain;
	Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;

	float clearColor[3] = { 0.0f, 0.0f, 0.0f };

private:
	bool InitializeDirectX();
	bool InitializeDXGI();
	bool InitializeDeviceAndContext();
	bool InitializeSwapchain();
	bool InitializeBackBuffRTV();
	bool InitializeDepthStencilView();
	bool InitializeRasterstate();
	bool InitializeSamplerState();
	bool InitializeViewportAndMatrices();

	void UpdateObjectConstantBuffers(IObject* const aObject);
	void UpdateShadowLightConstantBuffers(d3dLightClass* const aDirectionalLight);
	void UpdateFrameConstantBuffers(std::vector<std::unique_ptr<Light>>& aLights, d3dLightClass* const aDirectionalLight, Camera* const apCamera);
	
	void CreateConstantBuffers();

	bool DestroyDirectX();

	// Render functions
	void RenderObject(IObject* const aObject);
	void RenderMaterial(d3dMaterial* const aMaterial);
	void RenderFullScreenQuad();
	void RenderSceneDepthPrePass(std::vector<std::unique_ptr<IObject>>& aObjects);

	const float SCREEN_FAR = 1000.0f;
	const float SCREEN_NEAR = 2.0f;

	int gVideoCardMemoryAmount;
	char gVideoCardDescription[128];

	int gnumerator, gdenominator;

	HWND windowHandle;

	D3D11_VIEWPORT mViewport;
	D3D11_VIEWPORT mShadowLightViewport;


	XMFLOAT4X4 mProjectionMatrix;
	XMFLOAT4X4 mViewMatrix;


	// IDXGI stuff
	Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mpDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRaster_backcull;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRaster_frontcull;

	// Samplers
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mpAnisotropicWrapSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mpLinearClampSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mpPointClampSampler;
	
	// Constant buffers 
	std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
	std::unique_ptr<d3dConstantBuffer> mpLightCB;
	std::unique_ptr<d3dConstantBuffer> mpLightMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpPerObjectCB;

	// Shader manager
	std::unique_ptr<d3dShaderManager> mpShaderManager;

	// Render textures and render depth texture for shadow mapping
	std::unique_ptr<d3dRenderTexture> mBackBufferRenderTexture;
	std::unique_ptr<d3dRenderTexture> mSceneRenderTexture;
	std::unique_ptr<d3dRenderDepthTexture> mSceneDepthPrepassTexture;
};

