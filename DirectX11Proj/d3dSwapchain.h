#pragma once

#include <D3D11.h>
#include <wrl.h>

class d3dSwapchain
{
public:
	d3dSwapchain();
	~d3dSwapchain();

	bool Create(int aWidth, int aHeight, int aNumerator, int aDenominator, bool aVsyncEnabled, bool aFullScreen, HWND aHwnd);
	bool Create(DXGI_SWAP_CHAIN_DESC aSwapChainDesc);
	void Shutdown();
	void Swap(bool aIsVsync);

	IDXGISwapChain* const GetSwapChain();

private:
	// Creates the actual swap chain
	bool CreateSwapChainWithDesc(DXGI_SWAP_CHAIN_DESC);

	Microsoft::WRL::ComPtr<IDXGISwapChain> mpSwapchain;


};

