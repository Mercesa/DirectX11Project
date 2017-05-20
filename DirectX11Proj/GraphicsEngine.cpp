#include "GraphicsEngine.h"

#include "easylogging++.h"

#include "d3dShaderManager.h"

GraphicsEngine::GraphicsEngine()
{
}


GraphicsEngine::~GraphicsEngine()
{
}


bool GraphicsEngine::Initialize()
{
	return true;
}

bool GraphicsEngine::CreateDevice()
{
	HRESULT result;
	D3D_FEATURE_LEVEL featureLevel;
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	featureLevel = D3D_FEATURE_LEVEL_11_0;
	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, 0, 0, D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext);

	if (FAILED(result))
	{
		LOG(ERROR) << "device failed creation";
		return false;
	}

	return true;
}


ID3D11Device* const GraphicsEngine::GetDevice()
{
	return mpDevice.Get();
}


ID3D11DeviceContext* const GraphicsEngine::GetDeviceContext()
{
	return mpDeviceContext.Get();
}