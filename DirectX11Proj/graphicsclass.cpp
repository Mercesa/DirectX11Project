#include "graphicsclass.h"

#include "IScene.h"
#include "IObject.h"
#include "ModelLoader.h"
#include "ModelData.h"
#include "ResourceManager.h"
#include "d3dTexture.h"

GraphicsClass::GraphicsClass()
{
	mpDirect3D = 0;
	mpColorShader = 0;
	mTextureShader = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	// Create the Direct3D object.
	mpDirect3D = std::make_unique<D3DClass>();
	if(!mpDirect3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = mpDirect3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	ResourceManager::GetInstance().mpDevice = mpDirect3D->GetDevice();


	mpShaderManager = std::make_unique<d3dShaderManager>();

	if (!mpShaderManager->InitializeShaders(this->mpDirect3D->GetDevice()))
	{
		return false;
	}

	// Create the color shader object.
	mpColorShader = std::make_unique<ColorShaderClass>();
	if (!mpColorShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = mpColorShader->Initialize(mpDirect3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	mpColorShader->mpVertexShader = mpShaderManager->GetVertexShader("Shaders\\VS_color.hlsl");
	mpColorShader->mpPixelShader = mpShaderManager->GetPixelShader("Shaders\\PS_color.hlsl");

	// Create the color shader object.
	mTextureShader = std::make_unique<TextureShaderClass>();
	if (!mTextureShader)
	{
		return false;
	}
	
	// Initialize the color shader object.
	result = mTextureShader->Initialize(mpDirect3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	mTextureShader->mpVSShader = mpShaderManager->GetVertexShader("Shaders\\VS_texture.hlsl");
	mTextureShader->mpPSShader = mpShaderManager->GetPixelShader("Shaders\\PS_texture.hlsl");

	return true;
}


void GraphicsClass::Shutdown()
{
	mpColorShader->Shutdown();
	mTextureShader->Shutdown();
	mpDirect3D->Shutdown();
	
}

// Do culling
// Perform depth pre-pass
// Render objects to g-buffer
// Create ambient occlussion
// Render lighting on objects using the g-buffer
// Apply post processing effects
bool GraphicsClass::Frame(IScene *const apScene)
{
	bool result;
	static float f = 0.0f;
	result = Render(apScene);
	if(!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render(IScene *const aScene)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	mpDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	 aScene->GetCamera()->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	mpDirect3D->GetWorldMatrix(worldMatrix);
	aScene->GetCamera()->GetViewMatrix(viewMatrix);
	mpDirect3D->GetProjectionMatrix(projectionMatrix);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.

	for (int i = 0; i < aScene->mObjects.size(); ++i)
	{
		//std::cout << i << std::endl;
		aScene->mObjects[i]->mpModel->Render(mpDirect3D->GetDeviceContext());

		if (aScene->mObjects[i]->mpModel->mMaterial->mpDiffuse->exists)
		{
			mTextureShader->Render(mpDirect3D->GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, aScene->mObjects[i]->mpModel->mMaterial.get(), aScene->mLights, aScene->GetCamera()->GetPosition(), aScene->mDirectionalLight.get());
		}

		else
		{
			result = mpColorShader->Render(mpDirect3D->GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		}
	}


	// Present the rendered scene to the screen.
	mpDirect3D->EndScene();

	return true;
}