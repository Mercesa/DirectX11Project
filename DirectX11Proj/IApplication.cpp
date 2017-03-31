#include "IApplication.h"

#include "IScene.h"
#include "systemclass.h"

IApplication::IApplication() : currentScene(nullptr)
{
}


IApplication::~IApplication()
{
}

void IApplication::SetCurrentScene(IScene* aScene)
{

}

void IApplication::SceneTick()
{
	if (currentScene != nullptr)
	{
		currentScene->Tick();
	}
}