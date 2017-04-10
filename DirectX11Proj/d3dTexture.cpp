#include "d3dTexture.h"

#include "easylogging++.h"
#include <wrl.h>

d3dTexture::d3dTexture()
{
}


d3dTexture::~d3dTexture()
{
	//this->mpTexture.Get()->Release();
}


bool d3dTexture::Initialize(ID3D11Device* const aDevice, const WCHAR* aFilepath)
{
	HRESULT result;

	ID3D11ShaderResourceView* tShaderView = this->mpTexture.Get();

	result = D3DX11CreateShaderResourceViewFromFile(aDevice, aFilepath, NULL, NULL, this->mpTexture.GetAddressOf(), NULL);

	if (FAILED(result))
	{
		return false;
	}

	return true;

}

void d3dTexture::Shutdown()
{

}

ID3D11ShaderResourceView* const  d3dTexture::GetTexture()
{
	return mpTexture.Get();
}
