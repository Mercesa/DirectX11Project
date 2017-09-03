#include "EngineTimer.h"


#include <chrono>
#include <ctime>






EngineTimer::EngineTimer() : totalTime(double_t(0.0f)), deltaTime(0.0f)
{
	start = std::chrono::system_clock::now();
	end = std::chrono::system_clock::now();
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

	totalTime += (double_t)elapsed_seconds.count();
	deltaTime = (double_t)elapsed_seconds.count();
	start = std::chrono::system_clock::now();
}


double_t EngineTimer::GetDeltaTime()
{
	return deltaTime;
}


double_t EngineTimer::GetTotalTime()
{
	return totalTime;
}