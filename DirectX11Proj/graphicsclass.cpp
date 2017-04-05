#include "graphicsclass.h"

#include "IScene.h"
#include "ModelLoader.h"
#include "ModelData.h"

GraphicsClass::GraphicsClass()
{
	mpDirect3D = 0;
	mpModel = 0;
	m_ColorShader = 0;
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
	// Create the color shader object.
	m_ColorShader = std::make_unique<ColorShaderClass>();
	if (!m_ColorShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_ColorShader->Initialize(mpDirect3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the model object.
	mpModel = std::make_unique<ModelClass>();
	if (!mpModel)
	{
		return false;
	}

	// Initialize the model object.
	result = mpModel->Initialize(mpDirect3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	std::unique_ptr<ModelLoader> tMLoader = std::make_unique<ModelLoader>();
	tMLoader->LoadModel("Models\\Sponza\\Sponza.obj");

	std::unique_ptr<ModelClass> tD3DModel;
	for (auto &e : tMLoader->GetMeshesToBeProcessed())
	{
		tD3DModel = std::make_unique<ModelClass>();

		tD3DModel->Initialize(mpDirect3D->GetDevice(), e);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
		this->mModels.push_back(std::move(tD3DModel));
	}



	return true;
}


void GraphicsClass::Shutdown()
{
	
	m_ColorShader->Shutdown();	
	mpModel->Shutdown();
	mpDirect3D->Shutdown();
	
	return;
}

// Do culling
// Perform depth pre-pass
// Render objects to g-buffer
// Create ambient occlussion
// Render lighting on objects using the g-buffer
// Apply post processing effects
bool GraphicsClass::Frame(IScene *const aScene)
{
	bool result;
	static float f = 0.0f;
	result = Render(aScene);
	if(!result)
	{
		return false;
	}

	return true;
}

#include <iostream>
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

	for (int i = 0; i < this->mModels.size(); ++i)
	{
		//std::cout << i << std::endl;
		mModels[i]->Render(mpDirect3D->GetDeviceContext());
		result = m_ColorShader->Render(mpDirect3D->GetDeviceContext(), mModels[i]->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	}
	// Render the model using the color shader.
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	mpDirect3D->EndScene();

	return true;
}