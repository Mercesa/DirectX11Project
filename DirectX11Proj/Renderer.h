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
#include "IScene.h"

class d3dRenderTexture;

class Renderer
{
public:
	Renderer();
	~Renderer();
	
	void Initialize(HWND aHwnd);
	void RenderScene(IScene* const aScene);
	void Destroy();

	void OnResize();
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

	void UpdateObjectConstantBuffers(IObject* const aObject, IScene* const aScene);
	void UpdateFrameConstantBuffers(IScene* const aScene);
	void CreateConstantBuffers();

	bool DestroyDirectX();

	// Render functions
	void RenderObject(IObject* const aObject);

	const float SCREEN_FAR = 1000.0f;
	const float SCREEN_NEAR = 2.0f;

	int gVideoCardMemoryAmount;
	char gVideoCardDescription[128];


	int gnumerator, gdenominator;

	HWND windowHandle;

	D3D11_VIEWPORT gViewPort;

	XMFLOAT4X4 gProjectionMatrix;
	XMFLOAT4X4 gViewMatrix;


	Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mpDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mpAnisotropicWrapSampler;

	std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
	std::unique_ptr<d3dConstantBuffer> mpLightCB;

	std::unique_ptr<d3dShaderManager> mpShaderManager;

	std::unique_ptr<d3dRenderTexture> mBackBufferRenderTexture;

};

