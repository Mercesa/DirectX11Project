#pragma once

#include <d3d11.h>


#include <wrl.h>

class d3dDepthStencil
{
public:
	d3dDepthStencil(ID3D11Device* const aDevice, ID3D11DeviceContext* aContext);
	~d3dDepthStencil();

	bool Create(int aScreenWidth, int aScreenheight);

	ID3D11Texture2D* const GetDepthTexture();
	ID3D11DepthStencilState* const GetDepthStencilState();
	ID3D11DepthStencilView* const GetDepthStencilView();

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBufferTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;

	ID3D11Device* const mPDevice;
	ID3D11DeviceContext* const mPDeviceContext;
};

