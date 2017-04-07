#pragma once

#include <D3D11.h>
#include <wrl.h>

class d3dRasterizerState
{
public:
	d3dRasterizerState(ID3D11Device* const mPDevice, ID3D11DeviceContext* const aContext);
	~d3dRasterizerState();

	bool Create();

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;

private:
	ID3D11Device* const mPDevice;
	ID3D11DeviceContext* const mPDeviceContext;
};

