#include "PlayerSceneExample.h"

#include <iostream>

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
	x += aInput->mMouseRelX;
	y += aInput->mMouseRelY;

	std::cout << "xderp: " << x << std::endl;
	std::cout << "yderp: " << y << std::endl;
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
	mCamera->SetPosition(0.0f, 9.0f, 0.0f);
	
	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
