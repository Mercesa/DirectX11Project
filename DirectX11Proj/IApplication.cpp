#include "IApplication.h"

#include <iostream>

#include "easylogging++.h"

#include "IScene.h"
#include "systemclass.h"



IApplication::IApplication() : mShouldQuit(false)
{
}


IApplication::~IApplication()
{

}

// Setting the current scene also initializes it
void IApplication::LoadScene(std::unique_ptr<IScene> apScene)
{
	assert(apScene != nullptr);

	if (apScene == nullptr)
	{
		std::cout << "Can not set a nullptr as current scene!" << std::endl;
		return;
	}

	// Delete scene last used.
	if (mpCurrentScene != nullptr)
	{
		mpCurrentScene.release();
	}

	mpCurrentScene = std::move(apScene);;

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
