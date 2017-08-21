#include "IScene.h"

#include "IObject.h"

// Create camera on creation of scene
IScene::IScene() 
	: mInitialized(false)
{
	mpCamera = std::make_unique<Camera>();
	mpSkyboxSphere = std::make_unique<IObject>();
}


IScene::~IScene()
{
}
