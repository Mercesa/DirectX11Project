#pragma once

#include <cstdint>

#include "WindowsAndDXIncludes.h"

struct GPUTimeStampID
{
	explicit GPUTimeStampID(uint32_t aTimeStamp){}
	//explicit GPUTimeStampID(uint32_t aTimeStamp) {}

	uint32_t timestamp;
};

class GPUProfiler
{
public:
	GPUProfiler();
	~GPUProfiler();

	void WriteStatisticsToFile();

	void Initialize(ID3D11Device* const aDevice);
	void Cleanup();
	
	void BeginFrame(ID3D11DeviceContext* const aContext);
	void EndFrame(ID3D11DeviceContext* const aContext);

	void CollectData(ID3D11DeviceContext* const aContext);
	void SetStamp(ID3D11DeviceContext* const aContext);
	// Get Stamp

	// Get elapsed times
	
private:
	bool hasBeenInitialized;

	// This variable is here to ensure we don't insert timestamps when the render frame has not started
	bool hasStartedFrame;

	ID3D11Query* beginFrameQuery;
	ID3D11Query* endFrameQuery;
	ID3D11Query* disjoint;

	ID3D11Query* renderStatisticsQuery;

};

