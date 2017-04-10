#include "IScene.h"

#include "IObject.h"

// Create camera on creation of scene
IScene::IScene()
{
	mCamera = std::make_shared<CameraClass>();
}


IScene::~IScene()
{
}
