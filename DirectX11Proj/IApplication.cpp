#include "IApplication.h"

#include <iostream>


#include "IScene.h"
#include "systemclass.h"


IApplication::IApplication() : mpCurrentScene(nullptr), mShouldQuit(false)
{
}


IApplication::~IApplication()
{
	if (mpCurrentScene)
	{
		// fucking evil (wow). should really fix something about this (and think of a decent scene management system)
		delete mpCurrentScene;
	}
}

// Setting the current scene also initializes it
void IApplication::LoadScene(IScene* aScene)
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

	this->mpCurrentScene = aScene;
	if (!mpCurrentScene->HasBeenInitialized())
	{
		aScene->Init();
		aScene->mInitialized = true;
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
