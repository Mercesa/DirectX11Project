#include "PlayerSceneExample.h"

//#include <d3d11.h>
//#include "Inc\SimpleMath.h"
//using namespace DirectX::SimpleMath;
#include <iostream>

#include "ResourceManager.h"
#include "ObjectExample.h"
#include <windows.h>
#include <fcntl.h>


PlayerSceneExample::PlayerSceneExample()
{
}

PlayerSceneExample::~PlayerSceneExample()
{
}

void PlayerSceneExample::Tick(InputClass* const apInput, float aDT)
{
	//throw std::logic_error("The method or operation is not implemented.");
	static int x = 0;
	static int y = 0;

	
	int mouseRelX = 0;
	int mouseRelY = 0;
	apInput->GetMouseRelativeLocation(mouseRelX, mouseRelY);

	x = mouseRelX;
	y = mouseRelY;
	
	mpCamera->Pitch(y/360.0f);
	mpCamera->RotateY(x / 360.0f);
	
	if (apInput->IsKeyHeld(0x53))
	{
		mpCamera->Walk(-0.1f * aDT);
	}
	
	if (apInput->IsKeyHeld(0x57))
	{
		mpCamera->Walk(0.1f * aDT);
	}
	
	if (apInput->IsKeyHeld(0x41))
	{
		mpCamera->Strafe(-0.1f * aDT);
	}
	
	if (apInput->IsKeyHeld(0x44))
	{
		mpCamera->Strafe(0.1f * aDT);
	}
	//mpCamera->LookAt(XMFLOAT3(0.0f, 10.0f, -5.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	mpCamera->UpdateViewMatrix();
	//std::cout << mpCamera->GetPosition3f().z << std::endl;
}

void PlayerSceneExample::Init()
{
	mpCamera->SetPosition(0.0f, 2.0f, -1.0f);
	
	
	std::vector<ModelID> tModels;

	
	XMMATRIX tScaleMat = tScaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX tTranslateMat = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	// load models and translate them as game objects
	//for (int i = 0; i < tModels.size(); ++i)
	//{
	//	std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
	//	tpObject->mpModel = tModels[i];
	//	XMStoreFloat4x4(&tpObject->mWorldMatrix, tScaleMat);
	//
	//	this->mObjects.push_back(std::move(tpObject));
	//}

	// Load cube
	//tModels = ResourceManager::GetInstance().LoadModels("Models\\Box\\cube.obj");
	//tTranslateMat = XMMatrixTranslation(1.0f, 1.0f, 1.0f);
	//for (int i = 0; i < tModels.size(); ++i)
	//{
	//	std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
	//	tpObject->mpModel = tModels[i];
	//	XMStoreFloat4x4(&tpObject->mWorldMatrix, XMMatrixScaling(10.0, 1.0, 10.0f));
	//
	//	this->mObjects.push_back(std::move(tpObject));
	//}
	// Load sphere
	tModels = ResourceManager::GetInstance().LoadModels("Models\\Sponza\\Sponza.obj");
	tTranslateMat = XMMatrixTranslation(0.0f, 5.0f, 0.0f);
	XMMATRIX tRotateMat = XMMatrixRotationX(-3.14f/2.0f);
	for (int i = 0; i < tModels.size(); ++i)
	{
		std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
		tpObject->mpModel = tModels[i];
		XMStoreFloat4x4(&tpObject->mWorldMatrix, XMMatrixScaling(0.01f, 0.01f, 0.01f));
	
		this->mObjects.push_back(std::move(tpObject));
	}


	tModels = ResourceManager::GetInstance().LoadModels("Models\\Sponza\\Sponza.obj");
	tTranslateMat = XMMatrixTranslation(0.0f, 5.0f, 0.0f);
	//XMMATRIX tRotateMat = XMMatrixRotationX(-3.14f / 2.0f);
	for (int i = 0; i < tModels.size(); ++i)
	{
		std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
		tpObject->mpModel = tModels[i];
		XMStoreFloat4x4(&tpObject->mWorldMatrix, XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), XMMatrixTranslation(100.0f, 0.0f, 0.0f)));

		this->mObjects.push_back(std::move(tpObject));
	}
	// Create light, set diffuse and position, add light to list
	std::unique_ptr<Light> tpLight = std::make_unique<Light>();
	tpLight->diffuseColor = XMFLOAT3(1.0f, 0.0f, 1.0f);
	tpLight->position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	this->mLights.push_back(std::move(tpLight));

	mDirectionalLight = std::make_unique<d3dLightClass>();
	mDirectionalLight->SetLookAt(0.0f, 0.0f, 0.00f);
	mDirectionalLight->GenerateProjectionMatrix(-32.0f, -32.0f);
	mDirectionalLight->SetPosition(0.0f, 0.0f, 0.0f);
	mDirectionalLight->GenerateViewMatrix();
	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
