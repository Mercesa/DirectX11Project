#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include <directxmath.h>
#include <wrl.h>

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
	};

public:
	textureshaderclass();
	~textureshaderclass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, XMMATRIX&, XMMATRIX&, XMMATRIX&, ID3D11ShaderResourceView*);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&, ID3D11ShaderResourceView*);
	void RenderShader(ID3D11DeviceContext*, int);

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11SamplerState* mpSampleState;
};

