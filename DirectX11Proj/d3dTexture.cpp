#include "d3dTexture.h"

#include "easylogging++.h"
#include <wrl.h>

d3dTexture::d3dTexture() : exists(false)
{
}


d3dTexture::~d3dTexture()
{
}


bool d3dTexture::Initialize(ID3D11Device* const apDevice, const WCHAR* apFilepath)
{
	HRESULT result;

	ID3D11ShaderResourceView* tShaderView = mpTexture.Get();

	result = D3DX11CreateShaderResourceViewFromFile(apDevice, apFilepath, NULL, NULL, mpTexture.GetAddressOf(), NULL);

	if (FAILED(result))
	{
		this->exists = false;
		return false;
	}

	this->exists = true;
	return true;

}

void d3dTexture::Shutdown()
{

}

ID3D11ShaderResourceView* const  d3dTexture::GetTexture() const
{
	return mpTexture.Get();
}
