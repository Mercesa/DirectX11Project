#include "d3dConstantBuffer.h"

#include "easylogging++.h"

d3dConstantBuffer::d3dConstantBuffer(int aSize, void* aData, ID3D11Device* const aDevice) : mpDevice(aDevice)
{
	char* dd = (char *)aData;

	if (!dd)
	{
		dd = new char[aSize];
		ZeroMemory(dd, aSize);
	}

	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = dd;
	d.SysMemPitch = 0;
	d.SysMemSlicePitch = 0;

	
	// Create buffer with constant buffer tag
	
	// Trying to find the amount of padding based on the allocation size
	// https://en.wikipedia.org/wiki/Data_structure_alignment
	int32_t padding = (16 - (aSize % 16)) % 16;
	int32_t newSize = aSize + padding;

	aDevice->CreateBuffer(&CD3D11_BUFFER_DESC(newSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), &d, &mpBuffer);
	
	delete[] dd;
}


void d3dConstantBuffer::UpdateBuffer(void* data, ID3D11DeviceContext* const aContext)
{
	// Update the sub resource with data
	//aContext->UpdateSubresource(this->mpBuffer, 0, nullptr, data, 0, 0);

	D3D11_MAPPED_SUBRESOURCE res;

	// Enable the buffer to be written to
	HRESULT result = aContext->Map(mpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	if (!FAILED(result))
	{
		// Copy data
		memcpy(res.pData, data, res.DepthPitch);

		// Disable the buffer to be written to
		aContext->Unmap(mpBuffer, 0);
	}

	else
	{
		LOG(ERROR) << "Constant buffer failed to update buffer";
	}
}

d3dConstantBuffer::~d3dConstantBuffer()
{
	mpBuffer->Release();
}


ID3D11Buffer* d3dConstantBuffer::GetBuffer()
{
	return mpBuffer;
}