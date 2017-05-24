#ifndef _COLORSHADERCLASS_H_
#define _COLORSHADERCLASS_H_


#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11async.h>
#include <directxmath.h>
#include <fstream>

#include "d3dShaderVS.h"
#include "d3dShaderPS.h"

using namespace DirectX;
using namespace std;


class ColorShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	ColorShaderClass();
	ColorShaderClass(const ColorShaderClass&);
	~ColorShaderClass();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, const XMMATRIX&, const XMMATRIX& , const XMMATRIX&);

	d3dShaderVS* mpVertexShader;
	d3dShaderPS* mpPixelShader;

private:
	bool InitializeShader(ID3D11Device*, WCHAR*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&) const;
	void RenderShader(ID3D11DeviceContext*, int);


private:
	
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
};

#endif