
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_



#include <d3d11.h>
#include <d3dx10math.h>
#include <windowsx.h>
#include <wrl.h>

#include <fcntl.h>
#include <d3d11.h>	
#include <directxmath.h>

using namespace DirectX;

class d3dRenderTexture
{
public:
	d3dRenderTexture();
	d3dRenderTexture(const d3dRenderTexture&);
	~d3dRenderTexture();

	bool Initialize(ID3D11Device*const, int aTextureWidth, int aTextureHeight, float aScreenNear, float aScreenFar);
	bool InitializeWithBackbuffer(ID3D11Device* const aDevice, IDXGISwapChain* const aSwapChain, int, int, float, float);

	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView*const GetShaderResourceView();

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mpDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mpRenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mpDepthStencilBuffer;
	
	D3D11_VIEWPORT m_viewport;
};

#endif