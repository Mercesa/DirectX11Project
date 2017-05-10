#pragma once

#include "modelclass.h"

class IObject
{
public:
	IObject();
	virtual ~IObject();

	virtual void OnDestroy() = 0;
	virtual void OnInit() = 0;
	virtual void Tick() = 0;
	
	bool GetActive() { return mIsActive; }
	bool GetCastShadow() { return mCastShadow; }
	bool GetHasBeenInitialized() { return mHasBeenInitialized; }


	ModelClass* mpModel;

protected:
	// premature initialization just so I dont forget in general
	bool mIsActive = false;
	bool mCastShadow = false;
	bool mHasBeenInitialized = false;

};

