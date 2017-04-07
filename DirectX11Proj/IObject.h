#pragma once
class IObject
{
public:
	IObject();
	virtual ~IObject();

	virtual void OnDestroy() = 0;
	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	
	// premature initialization just so I dont forget in general
	bool mIsActive = false;
};

