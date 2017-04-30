#pragma once

#include <list>
#include <memory>
#include <vector>


#include "cameraclass.h"
#include "inputclass.h"

class IObject;

class IScene
{
public:
	friend class IApplication;
	IScene();
	~IScene();

	virtual void Tick(InputClass* const apInput) = 0;
	virtual void Init() = 0;
	virtual void Destroy() = 0;

	// Just a bit lazy and turn it into a vector, will convert back to a list later
	std::vector <std::unique_ptr<IObject>> mObjects;
	
	bool HasBeenInitialized() { return mInitialized; }

	CameraClass* const GetCamera() { return mpCamera.get(); }

protected:
	std::unique_ptr<CameraClass> mpCamera;

private:
	bool mInitialized;
};

