#pragma once

#include <d3d11.h>

// https://github.com/degenerated1123/GD3D11/blob/master/D3D11Engine/D3D11ConstantBuffer.cpp inspired by (pretty much copied from)
class d3dConstantBuffer
{
public:
	// Size of the struct, the data provided, the device
	d3dConstantBuffer(int aSize, void* aData, ID3D11Device* const aDevice);
	~d3dConstantBuffer();
	
	void UpdateBuffer(void* data, ID3D11DeviceContext* const aContext);

	ID3D11Buffer* GetBuffer();

private:
	ID3D11Device* const mpDevice;
	ID3D11Buffer* mpBuffer;
	int OriginalSize; // Buffersize must be a multiple of 16
};

