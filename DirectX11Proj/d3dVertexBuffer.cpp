#include "d3dVertexBuffer.h"

#include <iostream>

#include "d3d11HelperFile.h"

d3dVertexBuffer::d3dVertexBuffer() : mAmountOfElements(0)
{
}


d3dVertexBuffer::~d3dVertexBuffer()
{
	mBuffer->buffer->Release();
	mBuffer->buffer = nullptr;
}


bool d3dVertexBuffer::Initialize(ID3D11Device *const aDevice,void* aData, size_t aSizeInBytes, uint32_t aAmountOfElements, uint32_t aBufferFlag)
{
	mBuffer = std::make_unique<Buffer>();
	mBuffer->buffer = CreateSimpleBuffer(aDevice, aData, aSizeInBytes, aAmountOfElements, aBufferFlag, D3D11_USAGE_DEFAULT);
	mBuffer->amountOfElements = aAmountOfElements;

	return true;
}

ID3D11Buffer* const d3dVertexBuffer::GetBuffer()
{
	return mBuffer->buffer;
}

uint32_t d3dVertexBuffer::GetAmountOfElements()
{
	return mBuffer->amountOfElements;
}
