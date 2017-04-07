#include "d3dSwapchain.h"

#include <minwinbase.h>
#include <cassert>
#include <iostream>


d3dSwapchain::d3dSwapchain(IDXGIFactory* const aFactory, ID3D11Device* const aDevice) : mPFactory(aFactory), mDevice(aDevice)
{
	assert(aFactory != nullptr);
	assert(aDevice != nullptr);
}


d3dSwapchain::~d3dSwapchain()
{
}

void d3dSwapchain::Shutdown()
{
	if (mSwapChain)
	{
		mSwapChain->SetFullscreenState(false, NULL);
	}
	mSwapChain.Reset();
}


IDXGISwapChain* const d3dSwapchain::GetSwapChainPtr()
{
	// Can't return a swapchain that has no
	if (mSwapChain.Get() == nullptr)
	{
		assert(false | "TRYING TO OBTAIN EMPTY SWAPCHAIN");
		return nullptr;
	}

	return mSwapChain.Get();
}

// Just creates a description for now
bool d3dSwapchain::Create(int aWidth, int aHeight, int aNumerator, int aDenominator, bool aVsyncEnabled, bool aFullScreen, HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = aWidth;
	swapChainDesc.BufferDesc.Height = aHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (aVsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = aNumerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = aDenominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (aFullScreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;
	
	return CreateSwapChainWithDesc(swapChainDesc);
}


bool d3dSwapchain::Create(DXGI_SWAP_CHAIN_DESC aSwapChainDesc)
{
	return CreateSwapChainWithDesc(aSwapChainDesc);
}


bool d3dSwapchain::CreateSwapChainWithDesc(DXGI_SWAP_CHAIN_DESC aDesc)
{
	HRESULT result;
	if (mSwapChain.Get() == nullptr)
	{
		IDXGISwapChain* tSC = mSwapChain.Get();
		result = mPFactory->CreateSwapChain(mDevice, &aDesc, &mSwapChain);
		if (FAILED(result))
		{
			return false;
		}
		
		else
		{
			return true;
		}
	}

	else
	{
		std::cout << "Swap chain is already active" << std::endl;
		return false;
	}
	// Should never reach this but if it does we have a fail-safe
	return false;
}


void d3dSwapchain::Swap(bool aIsVsync)
{
	// Present the back buffer to the screen since rendering is complete.
	if (aIsVsync)
	{
		// Lock to screen refresh rate.
		mSwapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		mSwapChain->Present(0, 0);
	}
}