#pragma once
#include "IObject.h"
class ObjectExample :
	public IObject
{
public:
	ObjectExample();
	virtual ~ObjectExample();

	virtual void OnDestroy() override;
	virtual void OnInit() override;
	virtual void Tick() override;
};

