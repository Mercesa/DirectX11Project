#pragma once

#include <list>
#include <memory>
#include <vector>

#include "inputclass.h"
#include "LightStruct.h"
#include "IObject.h"
#include "Camera.h"
#include "d3dLightClass.h"



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
	
	std::unique_ptr<d3dLightClass> mDirectionalLight;

	bool HasBeenInitialized() { return mInitialized; }

	Camera* const GetCamera() { return mpCamera.get(); }

protected:
	std::unique_ptr<Camera> mpCamera;

private:
	bool mInitialized;
};

