#include "IApplication.h"

#include <iostream>


#include "IScene.h"
#include "systemclass.h"


IApplication::IApplication() : mShouldQuit(false)
{
}


IApplication::~IApplication()
{

}

// Setting the current scene also initializes it
void IApplication::LoadScene(std::unique_ptr<IScene> aScene)
{
	assert(aScene != nullptr);

	if (aScene == nullptr)
	{
		std::cout << "Can not set a nullptr as current scene!" << std::endl;
		return;
	}

	if (mpCurrentScene != nullptr)
	{
		std::cout << "There is already a scene loaded!" << std::endl;
		return;
	}

	this->mpCurrentScene = std::move(aScene);
	if (!mpCurrentScene->HasBeenInitialized())
	{
		mpCurrentScene->Init();
		mpCurrentScene->mInitialized = true;
	}
}


void IApplication::SceneTick(InputClass* const aInput)
{
	if (mpCurrentScene != nullptr)
	{
		mpCurrentScene->Tick(aInput);
	}
}


bool IApplication::ShouldQuit()
{
	return mShouldQuit;
}
