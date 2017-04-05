#pragma once

#include <list>
#include <memory>

#include "cameraclass.h"
#include "inputclass.h"

class IObject;

class IScene
{
public:
	friend class IApplication;
	IScene();
	~IScene();

	virtual void Tick(InputClass* const aInput) = 0;
	virtual void Init() = 0;
	virtual void Destroy() = 0;

	std::list <std::shared_ptr<IObject>> mObjects;
	
	bool HasBeenInitialized() { return mInitialized; }

	CameraClass* const GetCamera() { return mCamera.get(); }

protected:
	std::shared_ptr<CameraClass> mCamera;

private:
	bool mInitialized = false;
};

