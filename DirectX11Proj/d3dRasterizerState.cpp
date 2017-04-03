#include "d3dRasterizerState.h"

#include <cassert>


d3dRasterizerState::d3dRasterizerState(ID3D11Device* const aPDevice, ID3D11DeviceContext* const aContext) : mpDevice(aPDevice), mpDeviceContext(aContext)
{
}


d3dRasterizerState::~d3dRasterizerState()
{
}

bool d3dRasterizerState::Create()
{
	assert(mpDevice != nullptr);
	assert(mpDeviceContext != nullptr);

	HRESULT result;
	D3D11_RASTERIZER_DESC rasterDesc;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = mpDevice->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}