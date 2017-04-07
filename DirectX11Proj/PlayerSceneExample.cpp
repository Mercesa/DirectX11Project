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

void PlayerSceneExample::Tick(InputClass* const aInput)
{
	//throw std::logic_error("The method or operation is not implemented.");
	static int x = 0;
	static int y = 0;

	int mouseRelX = 0;
	int mouseRelY = 0;
	aInput->GetMouseRelLocation(mouseRelX, mouseRelY);

	x += mouseRelX;
	y += mouseRelY;

	mCamera->SetRotation(y, x, 0);

	if (aInput->mKeyboardState[DIK_W] & 0x80)
	{
		mCamera->m_positionX += mCamera->lookAt.x;
		mCamera->m_positionY += mCamera->lookAt.y;
		mCamera->m_positionZ += mCamera->lookAt.z;
	}

	if (aInput->mKeyboardState[DIK_S] & 0x80)
	{
		mCamera->m_positionX -= mCamera->lookAt.x;
		mCamera->m_positionY -= mCamera->lookAt.y;
		mCamera->m_positionZ -= mCamera->lookAt.z;
	}
}

void PlayerSceneExample::Init()
{
	mCamera->SetPosition(0.0f, 15.0f, 0.0f);
	
	std::vector<ModelClass*> tModels = ResourceManager::getInstance().LoadModels("Models\\Sponza\\Sponza.obj");

	for (int i = 0; i < tModels.size(); ++i)
	{
		std::shared_ptr<ObjectExample> tObject = std::make_shared<ObjectExample>();
		tObject->mpModel = tModels[i];
		this->mObjects.push_back(tObject);
		std::cout << "Going through models" << std::endl;
	}

	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
