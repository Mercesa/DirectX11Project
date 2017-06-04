
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_



#include <d3d11.h>
#include <d3dx10math.h>
#include <windowsx.h>
#include <wrl.h>

#include <fcntl.h>
#include <d3d11.h>	
#include <directxmath.h>
#include <cstdint>

using namespace DirectX;

class d3dRenderTexture
{
public:
	d3dRenderTexture();
	d3dRenderTexture(const d3dRenderTexture&);
	~d3dRenderTexture();

	bool Initialize(ID3D11Device* const aDevice, uint32_t aTextureWidth, uint32_t aTextureHeight, float aScreenNear, float aScreenFar);
	bool InitializeWithBackbuffer(ID3D11Device* const aDevice, IDXGISwapChain* const aSwapChain, uint32_t aTextureWidth, uint32_t aTextureHeight, float aScreenNear, float aScreenFar);

	void SetRenderTarget(ID3D11DeviceContext*const aRenderTarget);
	void ClearRenderTarget(ID3D11DeviceContext*const aDevice, float r, float g, float b, float a);
	ID3D11ShaderResourceView*const GetShaderResourceView();

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mpDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpShaderResourceView;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mpRenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mpDepthStencilBuffer;
	
	D3D11_VIEWPORT m_viewport;
};

#endif