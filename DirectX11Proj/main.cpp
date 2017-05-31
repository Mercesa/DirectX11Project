#include <vld.h>

//#include "systemclass.h"
#define _CRT_SECURE_NO_DEPRECATE

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <memory>
#include <cassert>

#include <windowsx.h>
#include <fcntl.h>
#include <d3d11.h>	
#include <directxmath.h>
#include <wrl.h>

using namespace DirectX;



// Include and initialize easy logging here, while its part of the System, NO file will ever include main.
// And its nice to have a central place for the logging library
#include "easylogging++.h"
#include "inputclass.h"
#include "GraphicsSettings.h"
#include "d3dConstantBuffer.h"
#include "ConstantBuffers.h"

#include "PlayerSceneExample.h"
#include "ResourceManager.h"
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"
#include "d3dMaterial.h"
#include "d3dShaderManager.h"
INITIALIZE_EASYLOGGINGPP


class WindowsProcessClass
{
public:
	WindowsProcessClass() {}
	~WindowsProcessClass() {}

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:

};

const float SCREEN_FAR = 1000.0f;
const float SCREEN_NEAR = 2.0f;

int gVideoCardMemoryAmount;
char gVideoCardDescription[128];


int gnumerator, gdenominator;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static WindowsProcessClass* ApplicationHandle = 0;

Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;


Microsoft::WRL::ComPtr<IDXGISwapChain> mpSwapchain;
Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;

Microsoft::WRL::ComPtr<ID3D11Texture2D> mpDepthStencilBufferTexture;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mpDepthStencilState;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mpDepthStencilView;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterState;
Microsoft::WRL::ComPtr<ID3D11SamplerState> mpAnisotropicWrapSampler;

std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
std::unique_ptr<d3dConstantBuffer> mpLightCB;

std::unique_ptr<d3dShaderManager> mpShaderManager;
HWND windowHandle;

D3D11_VIEWPORT gViewPort;

XMFLOAT4X4 gProjectionMatrix;
XMFLOAT4X4 gViewMatrix;

//XMFLOAT4X4 gOrthoMatrix;

std::unique_ptr<InputClass> mpInput;


void UpdateFrameConstantBuffers(IScene* const aScene);
void UpdateObjectConstantBuffers(IObject* const aObject, IScene* const aScene);

void CreateConstantBuffers()
{
	// Create constant buffers
	mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(MatrixBufferType), nullptr, mpDevice.Get());
	mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(MaterialBufferType), nullptr, mpDevice.Get());
	mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(LightBufferType), nullptr, mpDevice.Get());
	LOG(INFO) << "Constant buffers created";
}

bool InitializeDirectX();
bool DestroyDirectX();

LRESULT CALLBACK WindowsProcessClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{

	case WM_MOUSEMOVE:
	{
		int xPos = GET_X_LPARAM(lparam);
		int yPos = GET_Y_LPARAM(lparam);

		mpInput->MouseMove(xPos, yPos);

		return 0;
	}

	// Check if a key has been pressed on the keyboard.
	case WM_KEYDOWN:
	{
		// If a key is pressed send it to the input object so it can record that state.
		mpInput->KeyDown((unsigned int)wparam);
		return 0;
	}

	// Check if a key has been released on the keyboard.
	case WM_KEYUP:
	{
		// If a key is released then send it to the input object so it can unset the state for that key.
		mpInput->KeyUp((unsigned int)wparam);
		return 0;
	}


	// Any other messages send to the default message handler as our application won't make use of them.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}

}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}


		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}

void RenderScene(IScene* const aScene)
{
	float color[4]{ 0.6f, 0.6f, 0.6f, 1.0f };

	mpDeviceContext->RSSetViewports(1, &gViewPort);
	mpDeviceContext->RSSetState(mRasterState.Get());

	mpDeviceContext->ClearRenderTargetView(mpRenderTargetView.Get(), color);
	mpDeviceContext->ClearDepthStencilView(mpDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mpDepthStencilState.Get(), 1);
	mpDeviceContext->OMSetRenderTargets(1, mpRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

	aScene->GetCamera()->UpdateViewMatrix();

	d3dShaderVS*const tVS = mpShaderManager->GetVertexShader("Shaders\\VS_texture.hlsl");
	d3dShaderPS*const tPS = mpShaderManager->GetPixelShader("Shaders\\PS_texture.hlsl");

	UpdateFrameConstantBuffers(aScene);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);
	mpDeviceContext->PSSetShader(tPS->GetPixelShader(), NULL, 0);
	mpDeviceContext->PSSetSamplers(0, 1, mpAnisotropicWrapSampler.GetAddressOf());

	for (int i = 0; i < aScene->mObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(aScene->mObjects[i].get(), aScene);
		aScene->mObjects[i]->mpModel->Render(mpDeviceContext.Get());

		int indices = aScene->mObjects[i]->mpModel->GetIndexCount();

		d3dMaterial* const aMaterial = aScene->mObjects[i]->mpModel->mMaterial.get();

		ID3D11ShaderResourceView* aView = aMaterial->mpDiffuse->GetTexture();
		mpDeviceContext->PSSetShaderResources(0, 1, &aView);

		ID3D11ShaderResourceView* aView2 = aMaterial->mpSpecular->GetTexture();
		mpDeviceContext->PSSetShaderResources(1, 1, &aView2);

		ID3D11ShaderResourceView* aView3 = aMaterial->mpNormal->GetTexture();
		mpDeviceContext->PSSetShaderResources(2, 1, &aView3);
		// Set the vertex input layout.

		// Set the sampler state in the pixel shader.
		mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());



		// Render the triangle.
		mpDeviceContext->DrawIndexed(indices, 0, 0);
	}


	// Present the rendered scene to the screen.
	mpSwapchain->Present((GraphicsSettings::gIsVsyncEnabled ? 1 : 0), 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	std::unique_ptr<WindowsProcessClass> mWProc = std::make_unique<WindowsProcessClass>();
	mpInput = std::make_unique<InputClass>();
	mpInput->Initialize();



	
	// This is for the console window
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);

	// Also define a debug flag here, just for now. Its all right, dont worry about it. shhhhh
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;
	LPCWSTR applicationName;
	HINSTANCE m_hinstance;

	// Get an external pointer to this object.	
	ApplicationHandle = mWProc.get();

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	applicationName = L"Engine";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);


	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (GraphicsSettings::gIsApplicationFullScreen)
	{
		// Determine the resolution of the clients desktop screen.
		GraphicsSettings::gCurrentScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		GraphicsSettings::gCurrentScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)GraphicsSettings::gCurrentScreenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)GraphicsSettings::gCurrentScreenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;

		// Create the window with the screen settings and get the handle to it.
		windowHandle = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
			WS_OVERLAPPEDWINDOW,
			posX, posY, GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, NULL, NULL, m_hinstance, NULL);
	}
	else
	{
	
		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - GraphicsSettings::gCurrentScreenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - GraphicsSettings::gCurrentScreenHeight) / 2;
		// Create the window with the screen settings and get the handle to it.
		windowHandle = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
		WS_OVERLAPPEDWINDOW,
		posX, posY, GraphicsSettings::gCurrentScreenWidth, GraphicsSettings::gCurrentScreenHeight, NULL, NULL, m_hinstance, NULL);
	}



	// Bring the window up on the screen and set it as main focus.
	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle);
	SetFocus(windowHandle);

	// Hide the mouse cursor.
	ShowCursor(true);


	InitializeDirectX();
	CreateConstantBuffers();
	mpShaderManager = std::make_unique<d3dShaderManager>();
	mpShaderManager->InitializeShaders(mpDevice.Get());
	ResourceManager::GetInstance().mpDevice = mpDevice.Get();
	std::unique_ptr<PlayerSceneExample> mpPlayerScene = std::make_unique<PlayerSceneExample>();
	mpPlayerScene->Init();

	MSG msg;
	bool done = false;
	while (!done)
	{
		mpInput->Frame();
	
		// Handle the windows messages.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	
		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
	
		if (mpInput->IsEscapePressed())
		{
			done = true;
		}
		mpPlayerScene->Tick(mpInput.get(), 1.0f);
		RenderScene(mpPlayerScene.get());
	}



	DestroyDirectX();
	DestroyWindow(windowHandle);
	windowHandle = NULL;
	
	if (GraphicsSettings::gIsApplicationFullScreen)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the application instance.
	UnregisterClass(applicationName, m_hinstance);
	m_hinstance = NULL;
	
	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return 0;
}

bool InitializeDeviceAndContext()
{
	assert(mpDevice.Get() == nullptr);

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
		return false;
	}
	return true;
}

bool InitializeSwapchain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = GraphicsSettings::gCurrentScreenWidth;
	swapChainDesc.BufferDesc.Height = GraphicsSettings::gCurrentScreenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (GraphicsSettings::gIsVsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = gnumerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = gdenominator;
	}

	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = windowHandle;

	// Turn multi-sampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (GraphicsSettings::gIsApplicationFullScreen)
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

	HRESULT result;
	IDXGISwapChain* tSC = mpSwapchain.Get();
	result = mFactory->CreateSwapChain(mpDevice.Get(), &swapChainDesc, &mpSwapchain);

	
	if (FAILED(result))
	{
		LOG(INFO) << "Failed to create swapchain";
	}
	return true;
}

bool InitializeDXGI()
{
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	HRESULT result;
	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&mFactory);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to create DXGIFactory";
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = mFactory->EnumAdapters(0, &mAdapter);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to enumerate adapters";
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = mAdapter->EnumOutputs(0, &mAdapterOutput);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to enumerate primary adapter output";
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = mAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		LOG(FATAL) << "Failed to get displaymode list";
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = mAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)GraphicsSettings::gCurrentScreenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)GraphicsSettings::gCurrentScreenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = mAdapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	gVideoCardMemoryAmount = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);


	stringLength = 0;
	size_t lValue = (size_t)stringLength;

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&lValue, gVideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	return true;
}

bool InitializeBackBuffRTV()
{
	HRESULT result;

	ID3D11Texture2D* backBufferPtr;
	result = mpSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to get back buffer ptr from swapchain";
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = mpDevice->CreateRenderTargetView(backBufferPtr, NULL, &mpRenderTargetView);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to create render target view";
		return false;
	}

	// Release the backbuffer ptr
	backBufferPtr->Release();
	backBufferPtr = 0;

	return true;
}

bool InitializeDepthStencilView()
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	HRESULT result;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = GraphicsSettings::gCurrentScreenWidth;
	depthBufferDesc.Height = GraphicsSettings::gCurrentScreenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = mpDevice->CreateTexture2D(&depthBufferDesc, NULL, &mpDepthStencilBufferTexture);
	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create texture 2D";
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = mpDevice->CreateDepthStencilState(&depthStencilDesc, &mpDepthStencilState);
	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil state";
	}

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = mpDevice->CreateDepthStencilView(mpDepthStencilBufferTexture.Get(), &depthStencilViewDesc, &mpDepthStencilView);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil view";
	}
	return true;
}

bool InitializeRasterstate()
{
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = mpDevice->CreateRasterizerState(&rasterDesc, &mRasterState);
	if (FAILED(result))
	{
		LOG(INFO) << "Rasterizer state failed to initialize";
		return false;
	}

	return true;
}

bool DestroyDirectX()
{
	if (mpDevice.Get() != nullptr)
	{
		mpDevice.Reset();
		mpDevice = nullptr;
	}

	if (mpDeviceContext.Get() != nullptr)
	{
		mpDeviceContext.Reset();
		mpDeviceContext = nullptr;
	}

	if (mpRenderTargetView.Get() != nullptr)
	{
		mpRenderTargetView.Reset();
		mpRenderTargetView = nullptr;
	}
	
	return true;
}

bool InitializeSamplerState()
{
	//ID3D10Blob* errorMessage;
	D3D11_SAMPLER_DESC samplerDesc;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = mpDevice->CreateSamplerState(&samplerDesc, &mpAnisotropicWrapSampler);
	if (FAILED(result))
	{
		LOG(WARNING) << "Failed to sampler states";
		return false;
	}
	return true;
}

bool InitializeViewportAndMatrices()
{
	gViewPort.Width = (float)GraphicsSettings::gCurrentScreenWidth;
	gViewPort.Height = (float)GraphicsSettings::gCurrentScreenHeight;
	gViewPort.MinDepth = 0.0f;
	gViewPort.MaxDepth = 1.0f;
	gViewPort.TopLeftX = 0.0f;
	gViewPort.TopLeftY = 0.0f;

	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)GraphicsSettings::gCurrentScreenWidth / (float)GraphicsSettings::gCurrentScreenHeight;
	XMStoreFloat4x4(&gProjectionMatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_FAR));

	XMFLOAT3 upF3, pF3, LF3;
	XMVECTOR upVector, positionVector, lookAtVector;

	upF3.x = 0; upF3.y = 1; upF3.z = 0;
	pF3.x = 0; pF3.y = 1.0; pF3.z = -4;
	LF3.x = 0; LF3.y = 0; LF3.z = 0;

	upVector = XMLoadFloat3(&upF3);
	positionVector = XMLoadFloat3(&pF3);
	lookAtVector = XMLoadFloat3(&LF3);


	XMStoreFloat4x4(&gViewMatrix, XMMatrixLookAtLH(positionVector, lookAtVector, upVector));

	return true;
}

bool InitializeDirectX()
{
	if (!InitializeDeviceAndContext())
	{
		LOG(INFO) << "Failed to create device and/or context";
		return false;
	}
	LOG(INFO) << "Successfully initialized device and context";

	if (!InitializeDXGI())
	{
		LOG(INFO) << "Failed to create DXGI";
		return false;
	}
	LOG(INFO) << "Successfully initialized DXGI";

	if (!InitializeSwapchain())
	{
		LOG(INFO) << "Failed to create Swapchain";
		return false;
	}
	LOG(INFO) << "Successfully initialized Swapchain";	

	if (!InitializeBackBuffRTV())
	{
		LOG(INFO) << "Failed to create RTV with back buffer";
		return false;
	}
	LOG(INFO) << "Successfully initialized backbuffer rtv";

	if (!InitializeDepthStencilView())
	{
		LOG(INFO) << "failed to create DSV";
		return false;
	}
	LOG(INFO) << "Succesfully initialized dsv";

	if (!InitializeRasterstate())
	{
		LOG(INFO) << "Failed to create raster state";
		return false;
	}
	LOG(INFO) << "Succesfully initialized raster state";

	if (!InitializeSamplerState())
	{
		LOG(INFO) << "Failed to create raster state";
		return false;
	}
	LOG(INFO) << "Succesfully initialized raster state";

	if (!InitializeViewportAndMatrices())
	{
		LOG(INFO) << "Failed to create viewport";
		return false;
	}
	LOG(INFO) << "Successfully initialized viewp and matrices";

	LOG(INFO) << "Successfully initialized DirectX!";

	return true;
}

void UpdateFrameConstantBuffers(IScene* const aScene)
{
	// Update light constant buffer

	static LightBufferType* dataPtr = new LightBufferType();
	uint32_t bufferNumber;

	// Get a pointer to the data in the constant buffer.

	dataPtr->amountOfLights = static_cast<int>(aScene->mLights.size());

	for (int i = 0; i < aScene->mLights.size(); ++i)
	{
		dataPtr->arr[i].position = aScene->mLights[i]->position;
		dataPtr->arr[i].diffuseColor = aScene->mLights[i]->diffuseColor;
	}

	dataPtr->directionalLight.diffuseColor = aScene->mDirectionalLight->diffuseColor;
	dataPtr->directionalLight.specularColor = aScene->mDirectionalLight->specularColor;
	dataPtr->directionalLight.position = aScene->mDirectionalLight->position;


	mpLightCB->UpdateBuffer((void*)dataPtr, mpDeviceContext.Get());


	bufferNumber = 2;
	ID3D11Buffer* tBuff = mpLightCB->GetBuffer();

	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
}

void UpdateObjectConstantBuffers(IObject* const aObject, IScene* const aScene)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	static MaterialBufferType* dataPtr = new MaterialBufferType();
	uint32_t bufferNumber;

	// Update material
	d3dMaterial* const tMat = aObject->mpModel->mMaterial.get();

	dataPtr->hasDiffuse = (int)tMat->mpDiffuse->exists;
	dataPtr->hasSpecular = (int)tMat->mpSpecular->exists;
	dataPtr->hasNormal = (int)tMat->mpNormal->exists;


	mpMaterialCB->UpdateBuffer((void*)dataPtr, mpDeviceContext.Get());
	bufferNumber = 1;
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
	// End update material


	// Update matrix cb

	// very bad code but will fix later either way
	MatrixBufferType* dataPtr2 = new MatrixBufferType();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	worldMatrix = XMLoadFloat4x4(&aObject->mWorldMatrix);
	projectionMatrix = XMLoadFloat4x4(&gProjectionMatrix);
	aScene->GetCamera()->GetViewMatrix(viewMatrix);


	// Transpose the matrices to prepare them for the shader.
	XMMATRIX worldMatrix2 = XMMatrixTranspose(worldMatrix);
	XMMATRIX viewMatrix2 = XMMatrixTranspose(viewMatrix);
	XMMATRIX projectionMatrix2 = XMMatrixTranspose(projectionMatrix);

	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	dataPtr2->world = worldMatrix2;
	dataPtr2->view = viewMatrix2;
	dataPtr2->projection = projectionMatrix2;
	dataPtr2->gEyePosX = aScene->GetCamera()->GetPosition().x;
	dataPtr2->gEyePosY = aScene->GetCamera()->GetPosition().y;
	dataPtr2->gEyePosZ = aScene->GetCamera()->GetPosition().z;


	mpMatrixCB->UpdateBuffer((void*)dataPtr2, mpDeviceContext.Get());

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	tBuff = mpMatrixCB->GetBuffer();
	// finally set the constant buffer in the vertex shader with the updated values.
	mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);


	delete dataPtr2;
	// end update matrix cb
}