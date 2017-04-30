#include "PlayerSceneExample.h"

#include <iostream>

#include "ResourceManager.h"
#include "ObjectExample.h"

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

	if (apInput->mKeyboardState[DIK_W] & 0x80)
	{
		mpCamera->m_positionX += mpCamera->lookAt.x;
		mpCamera->m_positionY += mpCamera->lookAt.y;
		mpCamera->m_positionZ += mpCamera->lookAt.z;
	}

	if (apInput->mKeyboardState[DIK_S] & 0x80)
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

	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
