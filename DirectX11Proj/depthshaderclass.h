#pragma once

#include <d3d11.h>
#include <d3dx11async.h>
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"

class depthshaderclass
{
public:
	depthshaderclass();
	~depthshaderclass();

private:
	ID3D11InputLayout* mpLayout;
	ID3D11Buffer* mpMatrixBuffer;
};

