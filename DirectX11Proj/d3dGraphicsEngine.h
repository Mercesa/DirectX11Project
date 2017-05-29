#pragma once

#include <d3d11.h>	
#include <directxmath.h>
#include <wrl.h>
#include <string>
#include <stdlib.h>
#include <cstdio>
#include <array>

using namespace DirectX;


// Includes
#include "d3dShaderManager.h"

// Forward declares
class d3dVertexBuffer;
class d3dShaderVS;
class d3dShaderPS;
class IObject;
class d3dMaterial;

struct ApplicationInfo
{
	int mScreenWidth, mScreenHeight;
	bool mShouldVsync;
};

class d3dGraphicsEngine
{
public:
	d3dGraphicsEngine();
	virtual ~d3dGraphicsEngine();


	// Events that can happen
	bool SetWindowHandle(HWND aHWND);
	
	bool OnResize();
	
	//bool OnStartFrame();

	//bool OnEndFrame();

	bool Initialize();
	
	bool CreateDevice();
	bool CreateDXGI();
	bool CreateDXGIFactoryObject();
	bool CreateSwapchain();
	bool CreateRasterState();

	bool ShutDown();
	bool InitializeShaders();
	bool PresentFrame();
	bool EnumAdapters();

	bool MatchDisplayMode(int aWidth, int aHeight, int& aNumerator, int& aDenominator, int aNumModes, DXGI_MODE_DESC* aModeArray);

	//void DrawObject(IObject* const aObject, d3dMaterial* const aMaterial, ID3D11DeviceContext* const aContext);

	ID3D11RenderTargetView* const GetBackBufferRTV() { return mMainRTV.Get(); }
	ID3D11DepthStencilView* const GetBackBufferDSV() { return mMainDSV.Get(); }

	ID3D11DeviceContext* const GetDeviceContext();
	ID3D11Device* const GetDevice();
	IDXGISwapChain* const getSwapChain();

	void StartStandardTargets(float clearColor[4], D3D11_VIEWPORT aViewPort);

	bool CreateDepthStencil();
	void GetVideoCardInfo(char* cardName, int& memory);


	// Handles to get the shaders
	d3dShaderVS* const GetVertexShader(const char* aShaderPath);
	d3dShaderPS* const GetPixelShader( const char* aShaderPath);

	// Window handle

private:
	d3dGraphicsEngine(d3dGraphicsEngine const&) = delete;
	void operator=(d3dGraphicsEngine const&) = delete;


	// Video card stuff
	int mVideoCardMemoryAmount;
	char mVideoCardDescription;

	int numerator, denominator; 

	// Device and context
	Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;

	// DXGI stuff
	Microsoft::WRL::ComPtr<IDXGISwapChain> mpSwapchain;
	Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;


	// Shader manager
	std::unique_ptr<d3dShaderManager> mpShaderManager;

	// Backbuffer stuff
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mMainRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mMainDSV;
	
	// Depth stencil state, buffer and view
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBufferTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterState;


	HWND hwnd;

};

