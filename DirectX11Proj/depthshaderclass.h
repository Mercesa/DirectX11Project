#pragma once

#include <memory>
#include <vector>

#include <d3d11.h>
#include <d3dx11async.h>
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"
#include "d3dConstantBuffer.h"
#include "d3dMaterial.h"
#include "LightStruct.h"
#include "d3dLightClass.h"

class depthshaderclass
{
public:
	depthshaderclass();
	~depthshaderclass();

	bool Initialize(ID3D11Device* const aDevice);
	void Shutdown();
	bool Render(ID3D11DeviceContext*const aDeviceContext, int aIndexCount, const XMMATRIX& aWorldMatrix, const XMMATRIX& aViewmatrix, const XMMATRIX& aProjectionMatrix, d3dLightClass* const aLightClass);


	d3dShaderVS* mpVSShader;
	d3dShaderPS* mpPSShader;

private:
	bool InitializeShader(ID3D11Device* const aDevice);

	bool SetShaderParameters(ID3D11DeviceContext*, const XMMATRIX& aWorldMatrix, const XMMATRIX& aViewMatrix, const XMMATRIX& aProjectionMatrix, d3dLightClass* const aLight);
	void RenderShader(ID3D11DeviceContext* const aDeviceContext, int aIndexCount);

	std::unique_ptr<d3dConstantBuffer> mpLightMatrixCB;
};


