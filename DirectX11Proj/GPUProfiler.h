#pragma once

#include <cstdint>

#include "WindowsAndDXIncludes.h"


// A query stamp has a begin and end, also two variables to check if 
struct QueryStamp
{
	enum QueryStampStatesEnum {eNOMARKSET, eFIRSTMARKSSET, eBOTHMARKSSET};

	std::string name;
	ID3D11Query* beginMark;
	ID3D11Query* endMark;
	
	QueryStampStatesEnum currentState;
};

class GPUProfiler
{
public:
	GPUProfiler();
	~GPUProfiler();


	void Initialize(ID3D11Device* const aDevice);
	void Shutdown();
	
	// Functions for beginning and ending the frame
	void BeginFrame(ID3D11DeviceContext* const aContext);
	void EndFrame(ID3D11DeviceContext* const aContext);

	void CollectData(ID3D11DeviceContext* const aContext);

	// Set stamp by providing a name, the name 
	void SetStamp(ID3D11DeviceContext* const aContext, ID3D11Device* const aDevice, std::string aName);
	float GetTimeBetweenQueries(ID3D11DeviceContext* const aContext, ID3D11Query* const aBeginQuery, ID3D11Query* const aEndQuery, uint64_t aDisjointFrequency);

	bool recordStatsToFile;

private:
	QueryStamp* CreateQueryStamp(ID3D11DeviceContext* const aContext, ID3D11Device* const aDevice, std::string aName);
	void WriteStatisticsToFile();

	ID3D11Query* beginFrameQuery;
	ID3D11Query* endFrameQuery;
	ID3D11Query* disjoint;

	// Query ptr for the render statistics
	ID3D11Query* renderStatisticsQuery;

	// Current frame the GPU profiler is keeping track of, every collect data is seen as a frame
	int64_t currentFrame = 0;

	bool hasBeenInitialized;
	// This variable is here to ensure we don't insert timestamps when the render frame has not started
	bool hasStartedFrame;

	std::vector<std::unique_ptr<QueryStamp>> queryStamps;
};

