#pragma once

#include "d3d11HelperFile.h"
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
	XMFLOAT4X4 mWorldMatrix;
	bool mCastShadow = true;

protected:
	// premature initialization just so I dont forget in general
	bool mIsActive = false;
	bool mHasBeenInitialized = false;
		
};

