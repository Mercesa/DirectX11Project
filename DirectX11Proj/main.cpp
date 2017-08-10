#include <vld.h>

//#include "systemclass.h"
#define _CRT_SECURE_NO_DEPRECATE

#include <memory>
#include <cassert>

// Include and initialize easy logging here, while its part of the System, NO file will ever include main.
// And its nice to have a central place for the logging library
#define ELPP_DISABLE_DEFAULT_CRASH_HANDLING
#undef ELPP_DEBUG_ASSERT_FAILURE

#include "easylogging++.h"
#include "inputclass.h"
#include "GraphicsSettings.h"

#include <windowsx.h>
#include <wrl.h>
#include <fcntl.h>

#include "PlayerSceneExample.h"

INITIALIZE_EASYLOGGINGPP

#include "Renderer.h"
#include "ImguiImplementation.h"
#include "ResourceManager.h"
#include "GPUProfiler.h"

HWND windowHandle;

class WindowsProcessClass
{
public:
	WindowsProcessClass() {}
	~WindowsProcessClass() {}

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:

};



static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static WindowsProcessClass* ApplicationHandle = 0;


std::unique_ptr<InputClass> mpInput;
std::unique_ptr<PlayerSceneExample> mpPlayerScene;
std::unique_ptr<Renderer> mpRenderer;
std::unique_ptr<GPUProfiler> mpGPUProfiler;

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

	case WM_RBUTTONDOWN:
		mpInput->RMouseDown();
		return 0;
		break;

	case WM_RBUTTONUP:
		mpInput->RMouseUp();
		return 0;
		break;



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


extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	if (GraphicsSettings::gShowDebugWindow)
	{
		ImGui_ImplDX11_WndProcHandler(hwnd, umessage, wparam, lparam);
		return true;
	}
		

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

bool showTWindow = true;
#include <chrono>
static void ShowExampleMenuFile()
{
	ImGui::MenuItem("(dummy menu)", NULL, false, false);
	if (ImGui::MenuItem("New")) {}
	if (ImGui::MenuItem("Open", "Ctrl+O")) {}
	if (ImGui::BeginMenu("Open Recent"))
	{
		ImGui::MenuItem("fish_hat.c");
		ImGui::MenuItem("fish_hat.inl");
		ImGui::MenuItem("fish_hat.h");
		if (ImGui::BeginMenu("More.."))
		{
			ImGui::MenuItem("Hello");
			ImGui::MenuItem("Sailor");
			if (ImGui::BeginMenu("Recurse.."))
			{
				ShowExampleMenuFile();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S")) {}
	if (ImGui::MenuItem("Save As..")) {}
	ImGui::Separator();
	if (ImGui::BeginMenu("Options"))
	{
		static bool enabled = true;
		ImGui::MenuItem("Enabled", "", &enabled);
		ImGui::BeginChild("child", ImVec2(0, 60), true);
		for (int i = 0; i < 10; i++)
			ImGui::Text("Scrolling Text %d", i);
		ImGui::EndChild();
		static float f = 0.5f;
		static int n = 0;
		ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
		ImGui::InputFloat("Input", &f, 0.1f);
		ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Colors"))
	{
		for (int i = 0; i < ImGuiCol_COUNT; i++)
			ImGui::MenuItem(ImGui::GetStyleColName((ImGuiCol)i));
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Disabled", false)) // Disabled
	{
		IM_ASSERT(0);
	}
	if (ImGui::MenuItem("Checked", NULL, true)) {}
	if (ImGui::MenuItem("Quit", "Alt+F4")) {
		GraphicsSettings::gShowDebugWindow = false;
	}
}

void ShowTitleMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ShowExampleMenuFile();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Render();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	std::unique_ptr<WindowsProcessClass> mWProc = std::make_unique<WindowsProcessClass>();
	mpInput = std::make_unique<InputClass>();
	mpInput->Initialize();


	mpRenderer = std::make_unique<Renderer>();
	
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
	SetCapture(windowHandle);
	
	// Hide the mouse cursor.
	ShowCursor(true);


	mpRenderer->Initialize(windowHandle);
	ImGui_ImplDX11_Init(windowHandle, mpRenderer->mpDevice.Get(), mpRenderer->mpDeviceContext.Get());

	mpPlayerScene = std::make_unique<PlayerSceneExample>();
	mpPlayerScene->Init();
	
	mpGPUProfiler = std::make_unique<GPUProfiler>();
	mpGPUProfiler->Initialize(mpRenderer->mpDevice.Get());

	MSG msg;
	bool done = false;
	while (!done)
	{
		ImGui_ImplDX11_NewFrame();
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

		if (mpInput->IsKeyDown(VK_F5))
		{
			GraphicsSettings::gShowDebugWindow = !GraphicsSettings::gShowDebugWindow;
		}
		mpPlayerScene->Tick(mpInput.get(), 1.0f);
		Render();

	}

	ResourceManager::GetInstance().Shutdown();
	ImGui_ImplDX11_Shutdown();

	mpGPUProfiler->Shutdown();
	mpRenderer->DestroyDirectX();

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

void Render()
{
	mpGPUProfiler->BeginFrame(mpRenderer->mpDeviceContext.Get());

	bool hasBeenSelected = false;
	if (GraphicsSettings::gShowDebugWindow)
	{
		ShowTitleMenu();
		
		static float f = 0.0f;
		
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		
		for (int i = 0; i < mpPlayerScene->mLights.size(); ++i)
		{
			ImGui::ColorEdit3("color point light", (float*)&mpPlayerScene->mLights[i]->diffuseColor);
		
			ImGui::ColorEditMode(ImGuiColorEditMode_UserSelect);
			static float pos[3] = { 0.0, 0.0, 0.0 };
			ImGui::ColorEdit3("position point light", (float*)&pos);
			mpPlayerScene->mLights[i]->position = XMFLOAT3(pos);
		}
		float arr[3] = { 0.01f, 0.01f, 0.01f };
		ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
		ImGui::ColorEdit3("color directional light", (float*)&mpPlayerScene->mDirectionalLight->mDiffuseColor);
		//ImGui::ColorEdit3("position directional light", (float*)&mpPlayerScene->mDirectionalLight->mPosition);
		//ImGui::InputFloat3("PITCH   YAW    ROLL", (float*)&mpPlayerScene->mDirectionalLight->mPosition);
		ImGui::DragFloat3("Pitch Yaw Roll", (float*)&mpPlayerScene->mDirectionalLight->mPosition, 0.1f, 0.0f, 0.0f, "%.3f", 1.0f);

		mpPlayerScene->mDirectionalLight->GenerateViewMatrix();

	}

	mpRenderer->RenderScene(mpPlayerScene->mObjects, mpPlayerScene->mLights, mpPlayerScene->mDirectionalLight.get(), mpPlayerScene->GetCamera(), mpPlayerScene->GetSkyboxSphere());


	ImGui::Render();

	mpRenderer->mpDeviceContext->ClearState();

	mpRenderer->mpSwapchain->Present((GraphicsSettings::gIsVsyncEnabled ? 1 : 0), 0);

	mpGPUProfiler->EndFrame(mpRenderer->mpDeviceContext.Get());
	mpGPUProfiler->CollectData(mpRenderer->mpDeviceContext.Get());

}