#include "graphicsclass.h"

#include "easylogging++.h"

#include "IScene.h"
#include "IObject.h"
#include "ModelLoader.h"
#include "ModelData.h"
#include "ResourceManager.h"

#include "d3dTexture.h"
//#include "d3dDXGIManager.h"
#include "d3dDepthStencil.h"
#include "d3dRasterizerState.h"
#include "GraphicsEngine.h"


GraphicsClass::GraphicsClass()
{
	mpColorShader = 0;
	mTextureShader = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	HRESULT result;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Store the vsync setting.
	m_vsync_enabled = vsync;

	GraphicsEngine& engine = GraphicsEngine::getInstance();

	engine.Initialize(screenWidth, screenHeight, hwnd);


	

	// Get the pointer to the back buffer.
	//result = engine.GetSwapchain()->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	//if (FAILED(result))
	//{
	//	LOG(ERROR) << "failed to get back buffer ptr from swapchain";
	//	return false;
	//}

	// Create the render target view with the back buffer pointer.
	//result = engine.GetDevice()->CreateRenderTargetView(backBufferPtr, NULL, &mpRenderTargetView);
	//if (FAILED(result))
	//{
	//	LOG(ERROR) << "failed to create render target view";
	//	return false;
	//}
	//
	//// Release pointer to the back buffer as we no longer need it.
	//backBufferPtr->Release();
	//backBufferPtr = 0;
	return true;
}


void GraphicsClass::CreateConstantBuffers()
{
	// Create constant buffers
	mpMatrixCB = std::make_unique<d3dConstantBuffer>(sizeof(MatrixBufferType), nullptr, GetDevice());
	mpMaterialCB = std::make_unique<d3dConstantBuffer>(sizeof(MaterialBufferType), nullptr, GetDevice());
	mpLightCB = std::make_unique<d3dConstantBuffer>(sizeof(LightBufferType), nullptr, GetDevice());
	LOG(INFO) << "Constant buffers created";
}


void GraphicsClass::UpdateFrameConstantBuffers(ID3D11DeviceContext* const aDeviceContext, IScene* const aScene)
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


	mpLightCB->UpdateBuffer((void*)dataPtr, aDeviceContext);


	bufferNumber = 2;
	ID3D11Buffer* tBuff = mpLightCB->GetBuffer();

	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);

}

void GraphicsClass::UpdateObjectConstantBuffers(ID3D11DeviceContext* const aDeviceContext, IObject* const aObject, IScene* const aScene)
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


	mpMaterialCB->UpdateBuffer((void*)dataPtr, aDeviceContext);
	bufferNumber = 1;
	ID3D11Buffer* tBuff = mpMaterialCB->GetBuffer();
	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);
	// End update material


	// Update matrix cb

	// very bad code but will fix later either way
	MatrixBufferType* dataPtr2 = new MatrixBufferType();


	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	worldMatrix = XMLoadFloat4x4(&aObject->mWorldMatrix);
	GetProjectionMatrix(projectionMatrix);
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


	mpMatrixCB->UpdateBuffer((void*)dataPtr2, aDeviceContext);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	tBuff = mpMatrixCB->GetBuffer();
	// finally set the constant buffer in the vertex shader with the updated values.
	aDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &tBuff);
	aDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &tBuff);


	delete dataPtr2;
	// end update matrix cb
}



void GraphicsClass::BeginScene(float red, float green, float blue, float alpha)
{
	GraphicsEngine& engine = GraphicsEngine::getInstance();


	return;
}


ID3D11Device* GraphicsClass::GetDevice()
{
	return GraphicsEngine::getInstance().GetDevice();
}


ID3D11DeviceContext* GraphicsClass::GetDeviceContext()
{
	return GraphicsEngine::getInstance().GetDeviceContext();
}


void GraphicsClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = XMLoadFloat4x4(&mProjectionmatrix);
}


void GraphicsClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = XMLoadFloat4x4(&mOrthoMatrix);
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;


	// Initialize the Direct3D object.
	result = Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_FAR, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	ResourceManager::GetInstance().mpDevice = GetDevice();


	d3dShaderManager* const tShaderManager = GraphicsEngine::getInstance().GetShaderManager();

	
	// Prepare the color shader
	mpColorShader = std::make_unique<ColorShaderClass>();
	if (!mpColorShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = mpColorShader->Initialize(GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	mpColorShader->mpVertexShader = tShaderManager->GetVertexShader("Shaders\\VS_color.hlsl");
	mpColorShader->mpPixelShader = tShaderManager->GetPixelShader("Shaders\\PS_color.hlsl");

	// Prepare the texture shader
	mTextureShader = std::make_unique<TextureShaderClass>();
	if (!mTextureShader)
	{
		return false;
	}
	
	// Initialize the color shader object.
	result = mTextureShader->Initialize(GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}




	// Prepare depth shader
	this->mDepthShader = std::make_unique<depthshaderclass>();
	result = mDepthShader->Initialize(GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}



	mpD3LightClass = std::make_unique<d3dLightClass>();
	// Why does light need to be negative?
	mpD3LightClass->SetPosition(0.0f, -100.0f, 0.0f);
	mpD3LightClass->SetLookAt(0.0f, 0.0f, 0.1f);
	mpD3LightClass->GenerateProjectionMatrix(1.0f, 1000.0f);
	mpD3LightClass->GenerateViewMatrix();

	mpRenderTexture = std::make_unique<d3dRenderTexture>();
	mpRenderTexture->Initialize(GetDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_FAR);


	CreateConstantBuffers();
	
}


void GraphicsClass::Shutdown()
{
	mpColorShader->Shutdown();
	mTextureShader->Shutdown();
	mpRenderTexture->Shutdown();
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.


	mpRenderTargetView.Reset();
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

	ID3D11DeviceContext* const tpContext = GetDeviceContext();

	//mpRenderTexture->SetRenderTarget(tpContext);
	//tpContext->ClearDepthStencilView(mpRenderTexture->m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//mpD3LightClass->GetViewMatrix(viewMatrix);
	//mpD3LightClass->GetProjectionMatrix(projectionMatrix);
		// Generate the view matrix based on the camera's position.
	 //mpDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	//aScene->GetCamera()->GetViewMatrix(viewMatrix);
	//mpDirect3D->GetProjectionMatrix(projectionMatrix);
	//for (int i = 0; i < aScene->mObjects.size(); ++i)
	//{
	//	aScene->mObjects[i]->mpModel->Render(mpDirect3D->GetDeviceContext());
	//
	//	// Get the world matrix from the object, maybe set the shader stuff once instead of per object
	//	worldMatrix = XMLoadFloat4x4(&aScene->mObjects[i]->mWorldMatrix);
	//	if (aScene->mObjects[i]->mpModel->mMaterial->mpDiffuse->exists)
	//	{
	//		mTextureShader->Render(mpDirect3D->GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, aScene->mObjects[i]->mpModel->mMaterial.get(), aScene->mLights, aScene->GetCamera()->GetPosition(), aScene->mDirectionalLight.get());
	//	}
	//
	//	else
	//	{
	//		result = mpColorShader->Render(mpDirect3D->GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//	}
	//}


	aScene->GetCamera()->UpdateViewMatrix();

	// Clear the buffers to begin the scene.
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = 1.0f;




	aScene->GetCamera()->GetViewMatrix(viewMatrix);
	GetProjectionMatrix(projectionMatrix);
	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.

	UpdateFrameConstantBuffers(tpContext, aScene);
	for (int i = 0; i < aScene->mObjects.size(); ++i)
	{
		UpdateObjectConstantBuffers(tpContext, aScene->mObjects[i].get(), aScene);
		aScene->mObjects[i]->mpModel->Render(GetDeviceContext());

		// Get the world matrix from the object, maybe set the shader stuff once instead of per object
		worldMatrix = XMLoadFloat4x4(&aScene->mObjects[i]->mWorldMatrix);
		if (aScene->mObjects[i]->mpModel->mMaterial->mpDiffuse->exists)
		{
			mTextureShader->Render(GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), aScene->mObjects[i]->mpModel->mMaterial.get());
		}

		else
		{
			result = mpColorShader->Render(GetDeviceContext(), aScene->mObjects[i]->mpModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		}
	}


	// Present the rendered scene to the screen.
//	mpSwapchain->Present((VSYNC_ENABLED ? 1 : 0), 0);

	return true;
}