#include "textureshaderclass.h"
#include "ShaderHelperFunctions.h"

#include <iostream>
#include "easylogging++.h"

#include "d3dTexture.h"
#include "GraphicsEngine.h"
#include "d3dShaderManager.h"
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

	d3dShaderManager*const shaderManager = GraphicsEngine::getInstance().GetShaderManager();

	mpVSShader = shaderManager->GetVertexShader("Shaders\\VS_texture.hlsl");
	mpPSShader = shaderManager->GetPixelShader("Shaders\\PS_texture.hlsl");

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


bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, d3dMaterial* const aMaterial)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, aMaterial);
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
	D3D11_SAMPLER_DESC samplerDesc;



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


bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, d3dMaterial* const aMaterial)
{


	
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
