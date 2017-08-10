#pragma once

#include <cstdint>

#include "WindowsAndDXIncludes.h"

struct GPUTimeStampID
{
	explicit GPUTimeStampID(uint32_t aTimeStamp){}
	//explicit GPUTimeStampID(uint32_t aTimeStamp) {}

	uint32_t timestamp;
};

struct QueryStamp
{
	std::string name;
	ID3D11Query* beginMark;
	ID3D11Query* endMark;
	
	bool hasStarted = false;
	bool hasFinished = false;
};

class GPUProfiler
{
public:
	GPUProfiler();
	~GPUProfiler();


	void Initialize(ID3D11Device* const aDevice);
	void Shutdown();
	
	void BeginFrame(ID3D11DeviceContext* const aContext);
	void EndFrame(ID3D11DeviceContext* const aContext);

	void CollectData(ID3D11DeviceContext* const aContext);
	void SetStamp(ID3D11DeviceContext* const aContext, ID3D11Device* const aDevice, std::string aName);
	// Get Stamp

	// Get elapsed times
	
private:
	QueryStamp* CreateQueryStamp(ID3D11DeviceContext* const aContext, ID3D11Device* const aDevice, std::string aName);
	void WriteStatisticsToFile();

	ID3D11Query* beginFrameQuery;
	ID3D11Query* endFrameQuery;
	ID3D11Query* disjoint;

	// Query ptr for the render statistics
	ID3D11Query* renderStatisticsQuery;

	// Current frame the GPU profiler is keeping track of
	int64_t currentFrame = 0;

	bool hasBeenInitialized;
	// This variable is here to ensure we don't insert timestamps when the render frame has not started
	bool hasStartedFrame;

	std::vector<std::unique_ptr<QueryStamp>> queryStamps;
};

