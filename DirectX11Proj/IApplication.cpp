#include "IApplication.h"

#include <iostream>

#include "easylogging++.h"

#include "IScene.h"



IApplication::IApplication() : mShouldQuit(false)
{
}


IApplication::~IApplication()
{

}

// Setting the current scene also initializes it
void IApplication::LoadScene(std::unique_ptr<IScene> apScene)
{
	assert(apScene);

	if (apScene == nullptr)
	{
		std::cout << "Can not set a nullptr as current scene!" << std::endl;
		return;
	}

	mpCurrentScene = std::move(apScene);

	if (!mpCurrentScene->HasBeenInitialized())
	{
		mpCurrentScene->Init();
		mpCurrentScene->mInitialized = true;
	}
}


bool IApplication::ShouldQuit()
{
	return mShouldQuit;
}

IScene* const IApplication::GetCurrentScene()
{
	return mpCurrentScene.get(); 
}