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
	bool Render(ID3D11DeviceContext*, int, d3dMaterial* const aMaterial);


	d3dShaderVS* mpVSShader;
	d3dShaderPS* mpPSShader;

private:
	bool InitializeShader(ID3D11Device*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext*, d3dMaterial* const aMaterial);
	void RenderShader(ID3D11DeviceContext*, int);

	ID3D11SamplerState* mpSampleState;

	d3dConstantBuffer* mpMatrixCB;
	d3dConstantBuffer* mpMaterialCB;
	d3dConstantBuffer* mpLightCB;
};

