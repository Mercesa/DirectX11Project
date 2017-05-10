#pragma once

#include <vector>
#include "LightStruct.h"

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include <directxmath.h>
#include <wrl.h>

#include "d3dMaterial.h"

using namespace DirectX;
using namespace std;

#include "ConstantBuffers.h"
#include "d3dConstantBuffer.h"

#include "d3dShaderVS.h"
#include "d3dShaderPS.h"

class TextureShaderClass
{	
public:
	TextureShaderClass();
	~TextureShaderClass();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLights, XMFLOAT3 aCamPos, Light* const aLight);


	d3dShaderVS* mpVSShader;
	d3dShaderPS* mpPSShader;

private:
	bool InitializeShader(ID3D11Device*);
	void ShutdownShader();

	bool SetLightConstantBufferData(ID3D11DeviceContext* const aDeviceContext, std::vector<unique_ptr<Light>>& aLights, Light* const aLight);
	bool SetMaterialConstantBufferData(ID3D11DeviceContext* const aDeviceContext, d3dMaterial* const aMaterial);
	bool SetShaderParameters(ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLights, XMFLOAT3 aCamPos, Light* const aLight);
	void RenderShader(ID3D11DeviceContext*, int);

	ID3D11SamplerState* mpSampleState;

	std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
	std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
	std::unique_ptr<d3dConstantBuffer> mpLightCB;
};

