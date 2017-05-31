
//#include <vld.h>
//
////#include "systemclass.h"
//
//#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dx11.lib")
//#pragma comment(lib, "d3dx10.lib")
//#pragma comment(lib, "d3dcompiler.lib")
//
//#include <memory>
//#include <cassert>
//
//#include <windowsx.h>
//#include <fcntl.h>
//#include <d3d11.h>	
//#include <directxmath.h>
//#include <wrl.h>
//
//using namespace DirectX;
//
//
//
//// Include and initialize easy logging here, while its part of the System, NO file will ever include main.
//// And its nice to have a central place for the logging library
//#include "easylogging++.h"
//#include "inputclass.h"
//#include "IScene.h"
//#include "ResourceManager.h"
//#include "PlayerSceneExample.h"
//#include "d3dConstantBuffer.h"
//#include "ConstantBuffers.h"
//#include "textureshaderclass.h"
//#include "colorshaderclass.h"
//#include "depthshaderclass.h"
//#include "d3dMaterial.h"
//#include "d3dShaderManager.h"
//#include "d3dShaderVS.h"
//#include "d3dShaderPS.h"
//
//
//
//
//
//
//INITIALIZE_EASYLOGGINGPP

/*



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

std::unique_ptr<d3dShaderManager> mShaderManager;

// Depth stencil state, buffer and view
Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBufferTexture;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mpRenderTargetView;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> grasterState;
Microsoft::WRL::ComPtr<ID3D11SamplerState> mpAnisotropicWrapSampler;

D3D11_VIEWPORT gViewPort;

XMFLOAT4X4 gProjectionMatrix;
//XMFLOAT4X4 gOrthoMatrix;

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
void CreateConstantBuffers()
{
	// Create constant buffers
	mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(MatrixBufferType), nullptr, mpDevice.Get());
	mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(MaterialBufferType), nullptr, mpDevice.Get());
	mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(LightBufferType), nullptr, mpDevice.Get());
	LOG(INFO) << "Constant buffers created";
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	std::unique_ptr<WindowsProcessClass> mWProc = std::make_unique<WindowsProcessClass>();
	mShaderManager = std::make_unique<d3dShaderManager>();
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
	/*if (FULL_SCREEN)
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
	*/
//
//	// Place the window in the middle of the screen.
//	posX = (GetSystemMetrics(SM_CXSCREEN) - SCREEN_WIDTH) / 2;
//	posY = (GetSystemMetrics(SM_CYSCREEN) - SCREEN_HEIGHT) / 2;
//	// Create the window with the screen settings and get the handle to it.
//	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
//		WS_OVERLAPPEDWINDOW,
//		posX, posY, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, m_hinstance, NULL);
//	//}
//
//
//
//	// Bring the window up on the screen and set it as main focus.
//	ShowWindow(m_hwnd, SW_SHOW);
//	SetForegroundWindow(m_hwnd);
//	SetFocus(m_hwnd);
//
//	// Hide the mouse cursor.
//	ShowCursor(true);
//
//
//	MSG msg;
//	bool done;
//
//	// Initialize the message structure.
//	ZeroMemory(&msg, sizeof(MSG));
//
//
//
//	/************************************************************************/
//	/* DIRECTX INITIALIZATION                                                                     */
//	/************************************************************************/
//	assert(mpDevice.Get() == nullptr);
//
//	HRESULT result;
//	D3D_FEATURE_LEVEL featureLevel;
//	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
//
//#if defined(_DEBUG)
//	// If the project is in a debug build, enable the debug layer.
//	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
//	featureLevel = D3D_FEATURE_LEVEL_11_0;
//	result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, 0, 0, D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext);
//	ResourceManager::GetInstance().mpDevice = mpDevice.Get();
//
//	if (FAILED(result))
//	{
//		LOG(ERROR) << "Device Creation failed";
//	}
//
//	else
//	{
//		LOG(INFO) << "Created graphics device and context";
//	}
//	/************************************************************************/
//	/* END DIRECTX INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	/************************************************************************/
//	/* START DXGI INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//
//	
//	
//
//
//	
//	
//
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//
//	LOG(INFO) << "DXGI finished initialization";
//
//	/************************************************************************/
//	/* END DXGI INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	/************************************************************************/
//	/* START SWAPCHAIN INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	
//
//	
//	
//
//	
//	
//
//	
//	
//	
//
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//
//	
//	
//
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//
//	
//	
//
//	
//	
//
//	
//	
//
//	
//	/************************************************************************/
//	/* STOP SWAPCHAIN INITIALIZATION                                                                     */
//	/************************************************************************/
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//
//	/************************************************************************/
//	/* START DEPTH STENCIL INITIALIZATION                                                                     */
//	/************************************************************************/
//
//
//	
//	
//	
//
//	
//	
//
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//
//	
//	
//
//	
//	
//	
//	
//
//	
//	
//	
//
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//	
//
//	
//	
//
//	
//	
//	
//	
//
//	
//
//	
//	
//	
//	
//
//	LOG(INFO) << "Depth stencil state, view and texture successfully initialized";
//	/************************************************************************/
//	/* END DEPTHSTENCIL INITIALIZATION                                                                     */
//	/************************************************************************/
//
//
//	/************************************************************************/
//	/* START RASTERIZER STATE INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	
//
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	/************************************************************************/
//	/* START RASTERIZER STATE INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	// Setup the viewport for rendering.
//	
//	
//	
//	
//	
//	
//
//	
//	
//	
//	
//	//XMStoreFloat4x4(&gOrthoMatrix, XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, SCREEN_NEAR, SCREEN_FAR));
//
//	/************************************************************************/
//	/* END RASTERIZER STATE INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	/************************************************************************/
//	/* SAMPLER INITIALIZATION                                                                     */
//	/************************************************************************/
//
//
//
//	
//
//	/************************************************************************/
//	/* END SAMPLER INITIALIZATION                                                                     */
//	/************************************************************************/
//
//	CreateConstantBuffers();
//
//	
//	
//	LOG(INFO) << "Scene initialized";
//
//	mShaderManager = std::make_unique<d3dShaderManager>();
//	mShaderManager->InitializeShaders(mpDevice.Get());
//
//	// Loop until there is a quit message from the window or the user.

//
//	ResourceManager::GetInstance().mpDevice = nullptr;
//	// Show the mouse cursor.
//	ShowCursor(true);
//
//	// Fix the display settings if leaving full screen mode.
//	if (FULL_SCREEN)
//	{
//		ChangeDisplaySettings(NULL, 0);
//	}
//
//	mpDevice.Reset();
//	mpDevice = nullptr;
//	// Remove the window.

//	return 0;
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//