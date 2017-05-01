#include "PlayerSceneExample.h"

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

void PlayerSceneExample::Tick(InputClass* const apInput)
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
		mpCamera->m_positionX += mpCamera->lookAt.x;
		mpCamera->m_positionY += mpCamera->lookAt.y;
		mpCamera->m_positionZ += mpCamera->lookAt.z;
	}

	if (apInput->IsKeyDown(0x57))
	{
		mpCamera->m_positionX -= mpCamera->lookAt.x;
		mpCamera->m_positionY -= mpCamera->lookAt.y;
		mpCamera->m_positionZ -= mpCamera->lookAt.z;
	}
}

void PlayerSceneExample::Init()
{
	mpCamera->SetPosition(0.0f, 15.0f, 0.0f);
	
	std::vector<ModelClass*> tModels = ResourceManager::GetInstance().LoadModels("Models\\Sponza\\Sponza.obj");

	for (int i = 0; i < tModels.size(); ++i)
	{
		std::unique_ptr<ObjectExample> tpObject = std::make_unique<ObjectExample>();
		tpObject->mpModel = tModels[i];
		this->mObjects.push_back(std::move(tpObject));
	}


	std::unique_ptr<Light> tpLight = std::make_unique<Light>();
	tpLight->colour = XMFLOAT3(1.0f, 0.0f, 0.0f);
	tpLight->position = XMFLOAT3(0.0f, 5.0f, 0.0f);
	this->mLights.push_back(std::move(tpLight));
	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
