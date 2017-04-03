#include "Application.h"

#include "PlayerSceneExample.h"


Application::Application()
{
}


Application::~Application()
{
}

void Application::Init() 
{ 
	auto tScene = new PlayerSceneExample();
	LoadScene(tScene);
}

void Application::Tick() 
{ 
	/*std::cout << "User their application working" << std::endl;*/ 
}

void Application::Destroy() 
{}