#include "d3dVertexBuffer.h"

#include <iostream>


d3dVertexBuffer::d3dVertexBuffer() : mAmountOfElements(0)
{
}


d3dVertexBuffer::~d3dVertexBuffer()
{
	mBuffer.Reset();
}


bool d3dVertexBuffer::Initialize(ID3D11Device *const aDevice,void* aData, size_t aSizeInBytes, uint32_t aAmountOfElements, uint32_t aBufferFlag)
{
	if (aSizeInBytes == 0)
	{
		return false;
	}

	HRESULT result;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT; 
	vertexBufferDesc.ByteWidth = aSizeInBytes;
	vertexBufferDesc.BindFlags = (D3D11_BIND_FLAG)aBufferFlag;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = aData;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	
	result = aDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &mBuffer);

	if (FAILED(result))
	{
		std::cout << "Creation of vertex buffer has failed" << std::endl;
		return false;
	}

	mAmountOfElements = aAmountOfElements;

	return true;
}

ID3D11Buffer* const d3dVertexBuffer::GetBuffer()
{
	return mBuffer.Get();
}

uint32_t d3dVertexBuffer::GetAmountOfElements()
{
	return mAmountOfElements;
}
