#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dcompiler.lib")

//#define _XM_NO_INTRINSICS_

#include <d3d11.h>	
#include <directxmath.h>
using namespace DirectX;

#include <wrl.h>
#include <memory>


class d3dSwapchain;
class d3dDepthStencil;
class d3dRasterizerState;

class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

private:
	bool m_vsync_enabled;
	Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;

	XMMATRIX mProjectionmatrix;
	XMMATRIX mWorldMatrix;
	XMMATRIX mOrthoMatrix;

	std::unique_ptr<d3dSwapchain> mpSwapChain;
	std::unique_ptr<d3dDepthStencil> mpDepthStencil;
	std::unique_ptr<d3dRasterizerState> mpRasterizerState;
};

#endif