#pragma once

#include <D3D11.h>
#include <wrl.h>

#include <cstdint>
#include "d3d11HelperFile.h"
class d3dVertexBuffer
{
public:
	d3dVertexBuffer();
	~d3dVertexBuffer();

	bool Initialize(ID3D11Device* const aDevice, void* aData, size_t aSizeInBytes, uint32_t aAmountOfElements, uint32_t aBufferFlag);

	ID3D11Buffer* const GetBuffer();
	uint32_t GetAmountOfElements();

private:
	std::unique_ptr<Buffer> mBuffer;
	uint32_t mAmountOfElements;
};

