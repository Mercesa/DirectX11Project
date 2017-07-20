#include "EngineTimer.h"


#include <chrono>
#include <ctime>

std::chrono::time_point<std::chrono::system_clock> start, end;


static double totalTime = 0.0f;
static double deltaTime = 0.0f;


EngineTimer::EngineTimer()
{
}


EngineTimer::~EngineTimer()
{
}


void EngineTimer::Start()
{
	start = std::chrono::system_clock::now();
}


void EngineTimer::Update()
{
	std::chrono::duration<double> elapsed_seconds = start - end;

	end = start;
	start = std::chrono::system_clock::now();

	totalTime += elapsed_seconds.count();
	deltaTime = elapsed_seconds.count();
}


float_t EngineTimer::GetDeltaTime()
{
	return deltaTime;
}


float_t EngineTimer::GetTotalTime()
{
	return totalTime;
}