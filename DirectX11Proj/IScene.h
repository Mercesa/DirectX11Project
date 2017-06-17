#pragma once

#include <list>
#include <memory>
#include <vector>


#include "cameraclass.h"
#include "inputclass.h"
#include "LightStruct.h"
#include "IObject.h"

class IScene
{
public:
	friend class IApplication;
	IScene();
	~IScene();

	virtual void Tick(InputClass* const apInput, float aDT) = 0;
	virtual void Init() = 0;
	virtual void Destroy() = 0;

	std::vector <std::unique_ptr<IObject>> mObjects;
	std::vector <std::unique_ptr<Light>> mLights;
	
	std::unique_ptr<Light> mDirectionalLight;

	bool HasBeenInitialized() { return mInitialized; }

	CameraClass* const GetCamera() { return mpCamera.get(); }

protected:
	std::unique_ptr<CameraClass> mpCamera;

private:
	bool mInitialized;
};

