#pragma once

#include <list>
#include <memory>

class IObject;

class IScene
{
public:
	friend class IApplication;
	IScene();
	~IScene();

	virtual void Tick() = 0;
	virtual void Init() = 0;
	virtual void Destroy() = 0;

	std::list <std::shared_ptr<IObject>> mObjects;
	
	bool HasBeenInitialized() { return mInitialized; }

private:
	bool mInitialized = false;
};

