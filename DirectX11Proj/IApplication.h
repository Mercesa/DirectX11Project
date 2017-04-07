#pragma once

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

	bool GetShouldQuit();

private:
	void SceneTick();
	void SetCurrentScene(IScene* aScene);

	IScene* currentScene;

	bool shouldQuit = false;
};

