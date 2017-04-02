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

	bool ShouldQuit();

private:
	void SceneTick();
	void LoadScene(IScene* aScene);

	IScene* mCurrentScene;

	bool mShouldQuit = false;
};

