#include "IScene.h"

#include "IObject.h"

// Create camera on creation of scene
IScene::IScene() : mInitialized(false)
{
	mCamera = std::make_unique<CameraClass>();
}


IScene::~IScene()
{
}
