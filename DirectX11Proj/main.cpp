#include <vld.h>

//#include "systemclass.h"

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
#include "IScene.h"
#include "ResourceManager.h"
#include "PlayerSceneExample.h"
#include "d3dConstantBuffer.h"
#include "ConstantBuffers.h"


#include "d3dGraphicsEngine.h"
#include "d3dMaterial.h"
#include "d3dShaderManager.h"
#include "d3dShaderVS.h"
#include "d3dShaderPS.h"

//std::unique_ptr<d3dConstantBuffer> mpMatrixCB;
//std::unique_ptr<d3dConstantBuffer> mpMaterialCB;
//std::unique_ptr<d3dConstantBuffer> mpLightCB;


std::unique_ptr<d3dGraphicsEngine> mGengine;
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


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;



static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static WindowsProcessClass* ApplicationHandle = 0;

//ID3D11Device* mpDevice;
//ID3D11DeviceContext* mpDeviceContext;

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

void RenderScene(IScene* const aScene);


void UpdateFrameConstantBuffers(IScene* const aScene);
void UpdateObjectConstantBuffers(IObject* const aObject, IScene* const aScene);
void CreateConstantBuffers()
{
	// Create constant buffers
	//mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(MatrixBufferType), nullptr, mpDevice);
	//mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(MaterialBufferType), nullptr, mpDevice);
	//mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(LightBufferType), nullptr, mpDevice);
	LOG(INFO) << "Constant buffers created";
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	mGengine = std::make_unique<d3dGraphicsEngine>();
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
		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - SCREEN_WIDTH) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - SCREEN_HEIGHT) / 2;
		// Create the window with the screen settings and get the handle to it.
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
			WS_OVERLAPPEDWINDOW,
			posX, posY, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, m_hinstance, NULL);
	//}



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
	
	mGengine->SetWindowHandle(m_hwnd);
	mGengine->Initialize();

	mGengine->InitializeShaders();
	
	//ResourceManager::GetInstance().mpDevice = mpDevice;

	// Setup the viewport for rendering.
	gViewPort.Width = (float)SCREEN_WIDTH;
	gViewPort.Height = (float)SCREEN_HEIGHT;
	gViewPort.MinDepth = 0.0f;
	gViewPort.MaxDepth = 1.0f;
	gViewPort.TopLeftX = 0.0f;
	gViewPort.TopLeftY = 0.0f;

	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	XMStoreFloat4x4(&gProjectionMatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, SCREEN_NEAR, SCREEN_FAR));
	//XMStoreFloat4x4(&gOrthoMatrix, XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, SCREEN_NEAR, SCREEN_FAR));

	/************************************************************************/
	/* END RASTERIZER STATE INITIALIZATION                                                                     */
	/************************************************************************/

	/************************************************************************/
	/* SAMPLER INITIALIZATION                                                                     */
	/************************************************************************/


	HRESULT result;
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
	//result = mpDevice->CreateSamplerState(&samplerDesc, &mpAnisotropicWrapSampler);
	//if (FAILED(result))
	//{
	//	LOG(WARNING) << "Failed to sampler states";
	//	return false;
	//}

	/************************************************************************/
	/* END SAMPLER INITIALIZATION                                                                     */
	/************************************************************************/

	CreateConstantBuffers();

	std::unique_ptr<PlayerSceneExample> mpPlayerScene = std::make_unique<PlayerSceneExample>();
	mpPlayerScene->Init();

	LOG(INFO) << "Scene initialized";
	


	// Loop until there is a quit message from the window or the user.
	done = false;
	while (!done)
	{
		//mpInput->Frame();

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


	mpAnisotropicWrapSampler.Reset();
	//mGengine->ShutDown();
	//ResourceManager::GetInstance().mpDevice = nullptr;
	//ResourceManager::GetInstance().Shutdown();
	
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
	float color[4]{ 0.6f, 0.6f, 0.6f, 1.0f };

	mGengine->StartStandardTargets(color, gViewPort);

	aScene->GetCamera()->UpdateViewMatrix();
	



	//d3dShaderVS*const tVS = mGengine->GetVertexShader("Shaders\\VS_texture.hlsl");
	//d3dShaderPS*const tPS = mGengine->GetPixelShader("Shaders\\PS_texture.hlsl");

	//UpdateFrameConstantBuffers(aScene);
	
	// Set the vertex and pixel shaders that will be used to render this triangle.
	//mpDeviceContext->VSSetShader(tVS->GetVertexShader(), NULL, 0);
	//mpDeviceContext->PSSetShader(tPS->GetPixelShader(), NULL, 0);
	//mpDeviceContext->PSSetSamplers(0, 1, mpAnisotropicWrapSampler.GetAddressOf());

	for (int i = 0; i < aScene->mObjects.size(); ++i)
	{
		//UpdateObjectConstantBuffers(aScene->mObjects[i].get(), aScene);
		//aScene->mObjects[i]->mpModel->Render(mpDeviceContext);

		int indices = aScene->mObjects[i]->mpModel->GetIndexCount();

		d3dMaterial* const aMaterial = aScene->mObjects[i]->mpModel->mMaterial.get();

		ID3D11ShaderResourceView* aView = aMaterial->mpDiffuse->GetTexture();
		//mpDeviceContext->PSSetShaderResources(0, 1, &aView);

		ID3D11ShaderResourceView* aView2 = aMaterial->mpSpecular->GetTexture();
		//mpDeviceContext->PSSetShaderResources(1, 1, &aView2);

		ID3D11ShaderResourceView* aView3 = aMaterial->mpNormal->GetTexture();
		//mpDeviceContext->PSSetShaderResources(2, 1, &aView3);
		// Set the vertex input layout.

		// Set the sampler state in the pixel shader.
		//mpDeviceContext->IASetInputLayout(tVS->mpLayout.Get());

		// Render the triangle.
		//mpDeviceContext->DrawIndexed(indices, 0, 0);
	}


	// Present the rendered scene to the screen.
	mGengine->PresentFrame();
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


	//mpLightCB->UpdateBuffer((void*)dataPtr, mpDeviceContext);


	bufferNumber = 2;
	//ID3D11Buffer* tBuff = mpLightCB->GetBuffer();

	//mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
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


	//mpMaterialCB->UpdateBuffer((void*)dataPtr, mpDeviceContext);
	bufferNumber = 1;
	//ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	//mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
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


	//mpMatrixCB->UpdateBuffer((void*)dataPtr2, mpDeviceContext);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	//tBuff = mpMatrixCB->GetBuffer();
	// finally set the constant buffer in the vertex shader with the updated values.
	//mpDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	//mpDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);


	delete dataPtr2;
	// end update matrix cb
}