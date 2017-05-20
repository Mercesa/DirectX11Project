
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_



#include <d3d11.h>
#include <d3dx10math.h>



class d3dRenderTexture
{
public:
	d3dRenderTexture();
	d3dRenderTexture(const d3dRenderTexture&);
	~d3dRenderTexture();

	bool Initialize(ID3D11Device*, int, int, float, float);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();
	void GetProjectionMatrix(D3DXMATRIX&);
	void GetOrthoMatrix(D3DXMATRIX&);

	ID3D11DepthStencilView* m_depthStencilView;


private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11Texture2D* m_depthStencilBuffer;
	D3D11_VIEWPORT m_viewport;
	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_orthoMatrix;
};

#endif