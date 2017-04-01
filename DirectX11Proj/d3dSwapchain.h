#pragma once

#include <D3D11.h>
#include <wrl.h>
class d3dSwapchain
{
public:
	d3dSwapchain(IDXGIFactory* const aFactory, ID3D11Device* const aDevice);
	~d3dSwapchain();

	bool Create(int aWidth, int aHeight, int aNumerator, int aDenominator, bool aVsyncEnabled, bool aFullScreen, HWND hwnd);
	bool Create(DXGI_SWAP_CHAIN_DESC aSwapChainDesc);
	void Shutdown();

	IDXGISwapChain* const GetSwapChainPtr();

private:
	bool hasBeenCreated;

	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

	// Creates the actual swap chain
	bool CreateSwapChainWithDesc(DXGI_SWAP_CHAIN_DESC);

	IDXGIFactory* const mPFactory;
	ID3D11Device* const mDevice;
};

