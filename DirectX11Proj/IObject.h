#pragma once

#include "d3d11HelperFile.h"

#include <glm\gtx\common.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\common.hpp>

class IObject
{
public:
	IObject();
	virtual ~IObject();

	virtual void OnDestroy(){}
	virtual void OnInit(){}
	virtual void Tick(){}
	
	bool GetActive() { return mIsActive; }
	bool GetCastShadow() { return mCastShadow; }
	bool GetHasBeenInitialized() { return mHasBeenInitialized; }


	ModelID mpModel;
	Material* mpMaterial;

	glm::mat4x4 mWorldMatrix;
	glm::vec4 mSpherePosition;

	bool mCastShadow = true;

protected:
	// premature initialization just so I dont forget in general
	bool mIsActive = false;
	bool mHasBeenInitialized = false;
		
};

