#pragma once

#include <chrono>

class EngineTimer
{
public:
	EngineTimer();
	~EngineTimer();

	void Start();
	void Update();
	float_t GetDeltaTime();
	float_t GetTotalTime();
};


