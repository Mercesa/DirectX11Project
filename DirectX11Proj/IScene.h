#pragma once

#include <vector>

class IObject;

class IScene
{
public:
	IScene();
	~IScene();

	virtual void Tick() = 0;
	virtual void Init() = 0;
	virtual void Destroy() = 0;

	std::vector<IObject> mObjects;
};

