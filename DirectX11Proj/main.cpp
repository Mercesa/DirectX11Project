#include <vld.h>

//#include "systemclass.h"

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
#include "IScene.h"
#include "ResourceManager.h"
#include "PlayerSceneExample.h"
#include "d3dConstantBuffer.h"
#include "ConstantBuffers.h"
#include "textureshaderclass.h"

std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
std::unique_ptr<d3dConstantBuffer> mpLightCB;

std::unique_ptr<TextureShaderClass> mTextureShader;

INITIALIZE_EASYLOGGINGPP


class WindowsProcessClass
{
public:
	WindowsProcessClass() {}
	~WindowsProcessClass() {}

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:

};

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_FAR = 1000.0f;
const float SCREEN_NEAR = 2.0f;

int gVideoCardMemoryAmount;
char gVideoCardDescription[128];


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int gnumerator, gdenominator;


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static WindowsProcessClass* ApplicationHandle = 0;

Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;


Microsoft::WRL::ComPtr<IDXGISwapChain> mpSwapchain;
Microsoft::WRL::ComPtr<IDXGIFactory> mFactory;
Microsoft::WRL::ComPtr<IDXGIAdapter> mAdapter;
Microsoft::WRL::ComPtr<IDXGIOutput> mAdapterOutput;


// Depth stencil state, buffer and view
Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBufferTexture;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> grasterState;
Microsoft::WRL::ComPtr<ID3D11SamplerState> mpAnisotropicWrapSampler;

D3D11_VIEWPORT gViewPort;

XMFLOAT4X4 gProjectionMatrix;
XMFLOAT4X4 gOrthoMatrix;

std::unique_ptr<InputClass> mpInput;


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
void GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, gVideoCardDescription);
	memory = gVideoCardMemoryAmount;
	return;
}

void RenderScene(IScene* const aScene);


void UpdateFrameConstantBuffers(IScene* const aScene);
void UpdateObjectConstantBuffers(IObject* const aObject, IScene* const aScene);
void CreateConstantBuffers();


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
	
	int screenWidth, screenHeight;
	screenWidth = 0;
	screenHeight = 0;

	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;
	LPCWSTR applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

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
	if (FULL_SCREEN)
	{
		// Determine the resolution of the clients desktop screen.
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;

		// Create the window with the screen settings and get the handle to it.
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
			WS_OVERLAPPEDWINDOW,
			posX, posY, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, m_hinstance, NULL);
	}
	else
	{
		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - SCREEN_WIDTH) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - SCREEN_HEIGHT) / 2;
		// Create the window with the screen settings and get the handle to it.
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
			WS_OVERLAPPEDWINDOW,
			posX, posY, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, m_hinstance, NULL);
	}



	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(true);

	
	MSG msg;
	bool done;

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));



	/************************************************************************/
	/* DIRECTX INITIALIZATION                                                                     */
	/************************************************************************/
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
	ResourceManager::GetInstance().mpDevice = mpDevice.Get();

	if (FAILED(result))
	{
		LOG(ERROR) << "Device Creation failed";
	}

	else
	{
		LOG(INFO) << "Created graphics device and context";
	}
	/************************************************************************/
	/* END DIRECTX INITIALIZATION                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* START DXGI INITIALIZATION                                                                     */
	/************************************************************************/

	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;

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
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
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

	LOG(INFO) << "DXGI finished initialization";

	/************************************************************************/
	/* END DXGI INITIALIZATION                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* START SWAPCHAIN INITIALIZATION                                                                     */
	/************************************************************************/

	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (VSYNC_ENABLED)
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
	swapChainDesc.OutputWindow = m_hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (FULL_SCREEN)
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

	IDXGISwapChain* tSC = mpSwapchain.Get();
	mFactory->CreateSwapChain(mpDevice.Get(), &swapChainDesc, &mpSwapchain);

	LOG(INFO) << "Created swapchain";
	/************************************************************************/
	/* STOP SWAPCHAIN INITIALIZATION                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* START DEPTH STENCIL INITIALIZATION                                                                     */
	/************************************************************************/


	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = SCREEN_WIDTH;
	depthBufferDesc.Height = SCREEN_HEIGHT;
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
	result = mpDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilBufferTexture);
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
	result = mpDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
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

	result = mpDevice->CreateDepthStencilView(mDepthStencilBufferTexture.Get(), &depthStencilViewDesc, &mDepthStencilView);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil view";
	}

	LOG(INFO) << "Depth stencil state, view and texture successfully initialized";
	/************************************************************************/
	/* END DEPTHSTENCIL INITIALIZATION                                                                     */
	/************************************************************************/


	/************************************************************************/
	/* START RASTERIZER STATE INITIALIZATION                                                                     */
	/************************************************************************/

	D3D11_RASTERIZER_DESC rasterDesc;

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

	result = mpDevice->CreateRasterizerState(&rasterDesc, &grasterState);
	if (FAILED(result))
	{
		LOG(INFO) << "Rasterizer state failed to initialize";
	}

	else
	{
		LOG(INFO) << "Rasterizer state initialized";
	}
	/************************************************************************/
	/* START RASTERIZER STATE INITIALIZATION                                                                     */
	/************************************************************************/

	// Setup the viewport for rendering.
	gViewPort.Width = (float)screenWidth;
	gViewPort.Height = (float)screenHeight;
	gViewPort.MinDepth = 0.0f;
	gViewPort.MaxDepth = 1.0f;
	gViewPort.TopLeftX = 0.0f;
	gViewPort.TopLeftY = 0.0f;

	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;
	XMStoreFloat4x4(&gProjectionMatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_FAR));
	//XMStoreFloat4x4(&gOrthoMatrix, XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, SCREEN_NEAR, SCREEN_FAR));

	/************************************************************************/
	/* END RASTERIZER STATE INITIALIZATION                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* SAMPLER INITIALIZATION                                                                     */
	/************************************************************************/



	ID3D10Blob* errorMessage;
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
	result = mpDevice->CreateSamplerState(&samplerDesc, &mpAnisotropicWrapSampler);
	if (FAILED(result))
	{
		LOG(WARNING) << "Failed to sampler states";
		return false;
	}

	/************************************************************************/
	/* END SAMPLER INITIALIZATION                                                                     */
	/************************************************************************/

	std::unique_ptr<PlayerSceneExample> mpPlayerScene = std::make_unique<PlayerSceneExample>();
	mpPlayerScene->Init();


	// Loop until there is a quit message from the window or the user.
	done = false;
	while (!done)
	{
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

		mpInput->Frame();
		if (mpInput->IsEscapePressed())
		{
			done = true;
		}

	}

	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return 0;
}

void RenderScene(IScene* const aScene)
{
	float color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
	// Set the depth stencil state.
	mpDeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), 1);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	mpDeviceContext->OMSetRenderTargets(1, mpRenderTargetView.GetAddressOf(), mDepthStencilView.Get());

	mpDeviceContext->RSSetState(grasterState.Get());
	mpDeviceContext->RSSetViewports(1, &gViewPort);


	mpDeviceContext->ClearRenderTargetView(mpRenderTargetView.Get(), color);


	mpDeviceContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpDeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), 1);

	mpDeviceContext->OMSetRenderTargets(1, mpRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	


	aScene->GetCamera()->UpdateViewMatrix();




	//aScene->GetCamera()->GetViewMatrix(viewMatrix);
	//GetProjectionMatrix(projectionMatrix);
	//// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//
	//UpdateFrameConstantBuffers(tpContext, aScene);
	//for (int i = 0; i < aScene->mObjects.size(); ++i)
	//{
	//	UpdateObjectConstantBuffers(tpContext, aScene->mObjects[i].get(), aScene);
	//	aScene->mObjects[i]->mpModel->Render(GetDeviceContext());
	//
	//	// Get the world matrix from the object, maybe set the shader stuff once instead of per object
	//	worldMatrix = XMLoadFloat4x4(&aScene->mObjects[i]->mWorldMatrix);
	//	if (aScene->mObjects[i]->mpModel->mMaterial->mpDiffuse->exists)
	//	{
	//		mTextureShader->Render(GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), aScene->mObjects[i]->mpModel->mMaterial.get());
	//	}
	//
	//	else
	//	{
	//		result = mpColorShader->Render(GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//	}
	//}


	// Present the rendered scene to the screen.
	mpSwapchain->Present((VSYNC_ENABLED ? 1 : 0), 0);

}


void UpdateFrameConstantBuffers(IScene* const aScene)
{

	// Update light constant buffer

	HRESULT result;
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
	HRESULT result;
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