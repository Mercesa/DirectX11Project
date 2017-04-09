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
	auto tScene = std::make_unique<PlayerSceneExample>();
	LoadScene(std::move(tScene));
}

void Application::Tick() 
{ 
	/*std::cout << "User their application working" << std::endl;*/ 
}

void Application::Destroy() 
{}