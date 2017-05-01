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

class textureshaderclass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMFLOAT3 gEyePos;
	};

	struct MaterialBufferType
	{
		int hasDiffuse; // 4 bytes
		int hasSpecular;// 8 bytes
		int hasNormal;	// 12 bytes
		int padding0;	// 16 bytes, 16 bytes aligned :)
	};

	struct LightBufferType
	{
		int amountOfLights;
		int padLB01;
		int padLB02;
		int padLB03;

		Light arr[16];
	};

public:
	textureshaderclass();
	~textureshaderclass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, XMMATRIX&, XMMATRIX&, XMMATRIX&, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLights, XMFLOAT3 aCamPos);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*);
	void ShutdownShader();

	bool SetLightConstantBufferData(ID3D11DeviceContext* const aDeviceContext, std::vector<unique_ptr<Light>>& aLights);
	bool SetMaterialConstantBufferData(ID3D11DeviceContext* const aDeviceContext, d3dMaterial* const aMaterial);
	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLights, XMFLOAT3 aCamPos);
	void RenderShader(ID3D11DeviceContext*, int);

	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpPixelShader;
	ID3D11InputLayout* mpLayout;
	ID3D11Buffer* mpMatrixBuffer;
	ID3D11Buffer* mpMaterialConstantBuffer;
	ID3D11Buffer* mpLightConstantBuffer;
	ID3D11SamplerState* mpSampleState;
};

