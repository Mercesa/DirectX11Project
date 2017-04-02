#include "IApplication.h"

#include <iostream>


#include "IScene.h"
#include "systemclass.h"


IApplication::IApplication() : mCurrentScene(nullptr), mShouldQuit(false)
{
}


IApplication::~IApplication()
{
}

// Setting the current scene also initializes it
void IApplication::LoadScene(IScene* aScene)
{
	assert(aScene != nullptr);

	if (aScene == nullptr)
	{
		std::cout << "Can not set a nullptr as current scene!" << std::endl;
	}

	this->mCurrentScene = aScene;
	if (!mCurrentScene->HasBeenInitialized())
	{
		aScene->Init();
		aScene->mInitialized = true;
	}
}


void IApplication::SceneTick()
{
	if (mCurrentScene != nullptr)
	{
		mCurrentScene->Tick();
	}
}


bool IApplication::ShouldQuit()
{
	return mShouldQuit;
}
