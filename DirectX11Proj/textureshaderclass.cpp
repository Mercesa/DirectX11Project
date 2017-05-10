#include "textureshaderclass.h"
#include "ShaderHelperFunctions.h"

#include <iostream>
#include "easylogging++.h"

#include "d3dTexture.h"
TextureShaderClass::TextureShaderClass() 
{
	mpSampleState = 0;
}


TextureShaderClass::~TextureShaderClass()
{
}

bool TextureShaderClass::Initialize(ID3D11Device* device)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device);
	if (!result)
	{
		return false;
	}

	return true;
}


void TextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();
}


void TextureShaderClass::ShutdownShader()
{

	// Release the sampler state.
	if (mpSampleState)
	{
		mpSampleState->Release();
		mpSampleState = 0;
	}
}


bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLights, XMFLOAT3 aCamPos, Light* const aLight)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, aMaterial, aLights, aCamPos, aLight);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool TextureShaderClass::InitializeShader(ID3D11Device* device)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;



	mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(MatrixBufferType), nullptr, device);
	mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(MaterialBufferType), nullptr, device);
	mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(LightBufferType), nullptr, device); 

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &mpSampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool TextureShaderClass::SetLightConstantBufferData(ID3D11DeviceContext* const aDeviceContext, std::vector<unique_ptr<Light>>& aLights, Light* const aLight)
{
	HRESULT result;
	static LightBufferType* dataPtr = new LightBufferType();
	uint32_t bufferNumber;

	// Get a pointer to the data in the constant buffer.

	dataPtr->amountOfLights = static_cast<int>(aLights.size());

	

	for (int i = 0; i < aLights.size(); ++i)
	{
		dataPtr->arr[i].position = aLights[i]->position;
		dataPtr->arr[i].diffuseColor = aLights[i]->diffuseColor;
	}

	dataPtr->directionalLight.diffuseColor = aLight->diffuseColor;
	dataPtr->directionalLight.specularColor = aLight->specularColor;
	dataPtr->directionalLight.position = aLight->position;


	mpLightCB->UpdateBuffer((void*)dataPtr, aDeviceContext);


	bufferNumber = 2;
	ID3D11Buffer* tBuff = mpLightCB->GetBuffer();

	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

	return true;
}

bool TextureShaderClass::SetMaterialConstantBufferData(ID3D11DeviceContext* const aDeviceContext, d3dMaterial* const aMaterial)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	static MaterialBufferType* dataPtr = new MaterialBufferType();
	uint32_t bufferNumber;

	dataPtr->hasDiffuse = (int)aMaterial->mpDiffuse->exists;
	dataPtr->hasSpecular = (int)aMaterial->mpSpecular->exists;
	dataPtr->hasNormal = (int)aMaterial->mpNormal->exists;


	mpMaterialCB->UpdateBuffer((void*)dataPtr, aDeviceContext);
	bufferNumber = 1;
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

	return true;
}

bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLights, XMFLOAT3 aCamPos, Light* const aLight)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	static MatrixBufferType* dataPtr = new MatrixBufferType();
	uint32_t bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX worldMatrix2 = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrix2 = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrix2 = XMMatrixTranspose(projectionMatrix);


	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix2;
	dataPtr->view = viewMatrix2;
	dataPtr->projection = projectionMatrix2;
	dataPtr->gEyePos = aCamPos;

	mpMatrixCB->UpdateBuffer((void*)dataPtr, deviceContext);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	ID3D11Buffer* tBuff = mpMatrixCB->GetBuffer();
	// finally set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	SetMaterialConstantBufferData(deviceContext, aMaterial);
	SetLightConstantBufferData(deviceContext, aLights, aLight);
	
	
	
	ID3D11ShaderResourceView* aView = aMaterial->mpDiffuse->GetTexture();

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &aView);


	ID3D11ShaderResourceView* aView2 = aMaterial->mpSpecular->GetTexture();

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(1, 1, &aView2);


	ID3D11ShaderResourceView* aView3 = aMaterial->mpNormal->GetTexture();

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(2, 1, &aView3);


	return true;
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpVSShader->mpLayout.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVSShader->GetVertexShader(), NULL, 0);
	deviceContext->PSSetShader(mpPSShader->GetPixelShader(), NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpSampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
