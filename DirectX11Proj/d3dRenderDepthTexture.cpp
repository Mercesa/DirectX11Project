#include "d3dRenderDepthTexture.h"

#include "easylogging++.h"

#include "FormatConversionHelper.h"

d3dRenderDepthTexture::d3dRenderDepthTexture()
{
}


d3dRenderDepthTexture::~d3dRenderDepthTexture()
{
}

#include "d3d11HelperFile.h"

bool d3dRenderDepthTexture::Initialize(ID3D11Device* const aDevice, uint32_t aTextureWidth, uint32_t aTextureHeight, float aScreenNear, float aScreenFar)
{

	mpDepthTexture = CreateSimpleDepthTextureVisibleShader(aDevice, aTextureWidth, aTextureHeight);
	mpDepthStencilView = CreateSimpleDepthTextureShader(aDevice, mpDepthTexture.Get());
	mpShaderResourceView = CreateSimpleShaderResourceViewDepth(aDevice, mpDepthTexture.Get());
	

	return true;
}