#pragma once

#include "IApplication.h"

#include <iostream>

class Application : public IApplication
{
public:
	Application();
	~Application();

	virtual void Init()override;
	virtual void Tick()override;
	virtual void Destroy()override;
};

