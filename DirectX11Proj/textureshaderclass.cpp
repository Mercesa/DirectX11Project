#include "textureshaderclass.h"
#include "ShaderHelperFunctions.h"

#include <iostream>
#include "easylogging++.h"

#include "d3dTexture.h"
textureshaderclass::textureshaderclass() 
{
	mpVertexShader = 0;
	mpPixelShader = 0;
	mpLayout = 0;
	mpMatrixBuffer = 0;
	mpMaterialConstantBuffer = 0;
	mpSampleState = 0;
}


textureshaderclass::~textureshaderclass()
{
}

bool textureshaderclass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"texture.hlsl");
	if (!result)
	{
		return false;
	}

	return true;
}


void textureshaderclass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();
}


void textureshaderclass::ShutdownShader()
{

	// Release the sampler state.
	if (mpSampleState)
	{
		mpSampleState->Release();
		mpSampleState = 0;
	}

	// Release the matrix constant buffer.
	if (mpMatrixBuffer)
	{
		mpMatrixBuffer->Release();
		mpMatrixBuffer = 0;
	}

	if (mpMaterialConstantBuffer)
	{
		mpMaterialConstantBuffer->Release();
		mpMaterialConstantBuffer = 0;
	}

	if (mpLightConstantBuffer)
	{
		mpLightConstantBuffer->Release();
		mpLightConstantBuffer = 0;
	}

	// Release the layout.
	if (mpLayout)
	{
		mpLayout->Release();
		mpLayout = 0;
	}

	// Release the pixel shader.
	if (mpPixelShader)
	{
		mpPixelShader->Release();
		mpPixelShader = 0;
	}

	// Release the vertex shader.
	if (mpVertexShader)
	{
		mpVertexShader->Release();
		mpVertexShader = 0;
	}
}


bool textureshaderclass::Render(ID3D11DeviceContext* deviceContext, int indexCount, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLight, XMFLOAT3 aCamPos)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, aMaterial, aLight, aCamPos);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool textureshaderclass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	uint32_t numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	result = LoadShaderWithErrorChecking(vsFilename, "TextureVertexShader", "vs_5_0", vertexShaderBuffer, hwnd);

	if (!result)
	{
		return result;
	}

	result = LoadShaderWithErrorChecking(vsFilename, "TexturePixelShader", "ps_5_0", pixelShaderBuffer, hwnd);

	if (!result)
	{
		return result;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mpVertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpPixelShader);

	if (FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType structure in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;


	polygonLayout[3].SemanticName = "BITANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "TANGENT";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &mpLayout);
	
	if (FAILED(result))
	{
		LOG(WARNING) << "Failed to create input layout for textureshaderclass" << std::endl;
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;

	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &mpMatrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;

	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &mpMaterialConstantBuffer);
	if (FAILED(result))
	{
		return false;
	}

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(LightBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;

	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &mpLightConstantBuffer);
	if (FAILED(result))
	{
		return false;
	}





	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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

bool textureshaderclass::SetLightConstantBufferData(ID3D11DeviceContext* const aDeviceContext, std::vector<unique_ptr<Light>>& aLights)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	LightBufferType* dataPtr;
	uint32_t bufferNumber;


	result = aDeviceContext->Map(mpLightConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (LightBufferType*)mappedResource.pData;

	dataPtr->amountOfLights = static_cast<int>(aLights.size());

	for (int i = 0; i < aLights.size(); ++i)
	{
		dataPtr->arr[i].position = aLights[i]->position;
		dataPtr->arr[i].colour = aLights[i]->colour;
	}


	// Unlock the constant buffer.
	aDeviceContext->Unmap(mpLightConstantBuffer, 0);

	bufferNumber = 2;

	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightConstantBuffer);
}

bool textureshaderclass::SetMaterialConstantBufferData(ID3D11DeviceContext* const aDeviceContext, d3dMaterial* const aMaterial)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MaterialBufferType* dataPtr;
	uint32_t bufferNumber;

	
	result = aDeviceContext->Map(mpMaterialConstantBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MaterialBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->hasDiffuse = (int)aMaterial->mpDiffuse->exists;
	dataPtr->hasSpecular = (int)aMaterial->mpSpecular->exists;
	dataPtr->hasNormal = (int)aMaterial->mpNormal->exists;


	// Unlock the constant buffer.
	aDeviceContext->Unmap(mpMaterialConstantBuffer, 0);

	bufferNumber = 1;

	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpMaterialConstantBuffer);
}

bool textureshaderclass::SetShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
	const XMMATRIX& projectionMatrix, d3dMaterial* const aMaterial, std::vector<unique_ptr<Light>>& aLight, XMFLOAT3 aCamPos)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	uint32_t bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX worldMatrix2 = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrix2 = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrix2 = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix2;
	dataPtr->view = viewMatrix2;
	dataPtr->projection = projectionMatrix2;
	dataPtr->gEyePos = aCamPos;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finanly set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);
	SetMaterialConstantBufferData(deviceContext, aMaterial);
	SetLightConstantBufferData(deviceContext, aLight);
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

void textureshaderclass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpPixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpSampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

}
