#pragma once

#include <memory>
#include "inputclass.h"

class IScene;
class SystemClass;


// Application context struct maybe
// information about the application(size of window, name of window etc)
class IApplication
{
public:
	friend class SystemClass;

	IApplication();
	virtual ~IApplication();

	virtual void Init() = 0;
	virtual void Tick() = 0;
	virtual void Destroy() = 0;

	bool ShouldQuit();

	void LoadScene(std::unique_ptr<IScene> apScene);
	IScene* const GetCurrentScene();

private:
	std::unique_ptr<IScene> mpCurrentScene;

	bool mShouldQuit;
};

