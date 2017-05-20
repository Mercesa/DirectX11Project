#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <memory> 
#include <vector>

#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"
#include "textureshaderclass.h"
#include "d3dShaderManager.h"
#include "depthshaderclass.h"
#include "d3dRenderTexture.h"
#include "IObject.h"

#include <d3d11.h>	
#include <directxmath.h>
using namespace DirectX;

#include <wrl.h>
#include <memory>


class d3dSwapchain;
class d3dDepthStencil;
class d3dRasterizerState;

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 2.0f;

class IScene;

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(IScene *const);

private:
	bool Render(IScene* const);
	bool Initialize(int, int, bool, HWND, bool, float, float);

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void UpdateFrameConstantBuffers(ID3D11DeviceContext* const aDeviceContext, IScene* const aScene);
	void UpdateObjectConstantBuffers(ID3D11DeviceContext* const aDeviceContext, IObject* const aObject, IScene* const aScene);

	void CreateConstantBuffers();
private:

	// Constant buffers
	std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
	std::unique_ptr<d3dConstantBuffer> mpLightCB;
	
	std::unique_ptr<ColorShaderClass> mpColorShader;
	std::unique_ptr<TextureShaderClass> mTextureShader;
	std::unique_ptr<depthshaderclass> mDepthShader;

	std::unique_ptr<d3dShaderManager> mpShaderManager;

	std::unique_ptr<d3dLightClass> mpD3LightClass;
	std::unique_ptr<d3dRenderTexture> mpRenderTexture;

	bool m_vsync_enabled;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;

	XMFLOAT4X4  mProjectionmatrix;
	XMFLOAT4X4  mOrthoMatrix;

	std::unique_ptr<d3dSwapchain> mpSwapChain;
	std::unique_ptr<d3dDepthStencil> mpDepthStencil;
	std::unique_ptr<d3dRasterizerState> mpRasterizerState;

};
	
#endif