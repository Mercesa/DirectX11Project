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

	x += mouseRelX;
	y += mouseRelY;

	mpCamera->SetRotation(y, x, 0);
	
	if (apInput->IsKeyDown(0x53))
	{
		mpCamera->m_positionZ += -100.0f * aDT;
	}

	if (apInput->IsKeyDown(0x57))
	{
		mpCamera->m_positionZ += 100.0f * aDT;
	}

	if (apInput->IsKeyDown(0x41))
	{
		mpCamera->m_positionY -= 100.0f * aDT;
	}

	if (apInput->IsKeyDown(0x44))
	{
		mpCamera->m_positionY += 100.0f * aDT;
	}

}

void PlayerSceneExample::Init()
{
	mpCamera->SetPosition(0.0f, 15.0f, -100.0f);
	
	
	std::vector<ModelClass*> tModels = ResourceManager::GetInstance().LoadModels("Models\\Box\\cube.obj");

	
	XMMATRIX tScaleMat = tScaleMat = XMMatrixScaling(200.0f, 1.0f, 200.0f);
	XMMATRIX tTranslateMat = XMMatrixTranslation(0.0f, -5.0f, 0.0f);

	// load models and translate them as game objects
	for (int i = 0; i < tModels.size(); ++i)
	{
		std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
		tpObject->mpModel = tModels[i];
		XMStoreFloat4x4(&tpObject->mWorldMatrix, tScaleMat);

		this->mObjects.push_back(std::move(tpObject));
	}

	// Load cube
	tModels = ResourceManager::GetInstance().LoadModels("Models\\Box\\cube.obj");
	tTranslateMat = XMMatrixTranslation(25.0f, 25.0f, 0.0f);
	for (int i = 0; i < tModels.size(); ++i)
	{
		std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
		tpObject->mpModel = tModels[i];
		XMStoreFloat4x4(&tpObject->mWorldMatrix, XMMatrixMultiply(XMMatrixScaling(10.0f, 10.0f, 10.0f), tTranslateMat));

		this->mObjects.push_back(std::move(tpObject));
	}

	// Load sphere
	tModels = ResourceManager::GetInstance().LoadModels("Models\\Sphere\\sphere.obj");
	tTranslateMat = XMMatrixTranslation(0.0f, 25.0f, 0.0f);
	for (int i = 0; i < tModels.size(); ++i)
	{
		std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
		tpObject->mpModel = tModels[i];
		XMStoreFloat4x4(&tpObject->mWorldMatrix, XMMatrixMultiply(XMMatrixScaling(10.0f, 10.0f, 10.0f), tTranslateMat ));

		this->mObjects.push_back(std::move(tpObject));
	}

	// Create light, set diffuse and position, add light to list
	std::unique_ptr<Light> tpLight = std::make_unique<Light>();
	tpLight->diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	tpLight->position = XMFLOAT3(0.0f, 20, 0.0f);
	this->mLights.push_back(std::move(tpLight));

	this->mDirectionalLight = std::make_unique<Light>();
	this->mDirectionalLight->position = XMFLOAT3(0.0f, -1.0f, 0.5f);
	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
