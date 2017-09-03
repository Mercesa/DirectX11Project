#pragma once

#include <chrono>

class EngineTimer
{
public:
	EngineTimer();
	~EngineTimer();

	void Start();
	void Update();
	double_t GetDeltaTime();
	double_t GetTotalTime();

private:
	// Start and end point of timing
	std::chrono::time_point<std::chrono::system_clock> start, end;

	double totalTime;
	double deltaTime;

	

};


