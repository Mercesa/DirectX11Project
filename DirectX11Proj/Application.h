#pragma once

#include "IApplication.h"

#include <iostream>

class Application : public IApplication
{
public:
	Application();
	~Application();

	virtual void Init() {}
	virtual void Tick() { /*std::cout << "User their application working" << std::endl;*/ }
	virtual void Destroy() {}
};

