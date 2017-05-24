#include "depthshaderclass.h"

#include <cassert>
#include "ConstantBuffers.h"

#include "GraphicsEngine.h"
#include "d3dShaderManager.h"
depthshaderclass::depthshaderclass()
{
}


depthshaderclass::~depthshaderclass()
{
}

bool depthshaderclass::Initialize(ID3D11Device* const aDevice)
{

	d3dShaderManager*const shaderManager = GraphicsEngine::getInstance().GetShaderManager();

	mpVSShader = shaderManager->GetVertexShader("Shaders\\VS_depth.hlsl");
	mpPSShader = shaderManager->GetPixelShader("Shaders\\PS_depth.hlsl");
	if (!InitializeShader(aDevice))
	{
		return false;
	}

	return true;
}


void depthshaderclass::Shutdown()
{

}


bool depthshaderclass::InitializeShader(ID3D11Device* const aDevice)
{
	mpLightMatrixCB = std::make_unique<d3dConstantBuffer>((int)sizeof(LightMatrixBufferType), nullptr, aDevice);
	return true;
}

bool depthshaderclass::Render(ID3D11DeviceContext*const aDeviceContext, int aIndexCount, const XMMATRIX& aWorldMatrix, const XMMATRIX& aViewmatrix, const XMMATRIX& aProjectionMatrix, d3dLightClass* const aLightClass)
{
	SetShaderParameters(aDeviceContext, aWorldMatrix, aViewmatrix, aProjectionMatrix, aLightClass);

	RenderShader(aDeviceContext, aIndexCount);

	return true;
}

bool depthshaderclass::SetShaderParameters(ID3D11DeviceContext* const aDeviceContext, const XMMATRIX& aWorldMatrix, const XMMATRIX& aViewMatrix, const XMMATRIX& aProjectionMatrix, d3dLightClass* const aLight)
{
	static LightMatrixBufferType* dataPtr = new LightMatrixBufferType();
	
	dataPtr->worldMatrix = XMMatrixTranspose(aWorldMatrix);
	dataPtr->lightProjectionMatrix = XMMatrixTranspose(aProjectionMatrix);
	dataPtr->lightViewMatrix = XMMatrixTranspose(aViewMatrix);

	mpLightMatrixCB->UpdateBuffer((void*)dataPtr, aDeviceContext);

	ID3D11Buffer* const tpBuffer = mpLightMatrixCB->GetBuffer();
	aDeviceContext->VSSetConstantBuffers(3, 0, &tpBuffer);

	return true;
}

void depthshaderclass::RenderShader(ID3D11DeviceContext* const aDeviceContext, int aIndexCount)
{
	assert(mpVSShader);

	aDeviceContext->IASetInputLayout(mpVSShader->mpLayout.Get());
	aDeviceContext->VSSetShader(mpVSShader->GetVertexShader(), NULL, 0);
	aDeviceContext->PSSetShader(mpPSShader->GetPixelShader(), NULL, 0);

}