#include "GPUProfiler.h"



GPUProfiler::GPUProfiler() :
	hasBeenInitialized(false),
	hasStartedFrame(false),
	beginFrameQuery(nullptr),
	endFrameQuery(nullptr),
	disjoint(nullptr)
{
}
#include <sstream>
#include <fstream>
std::stringstream tempBuffer;
GPUProfiler::~GPUProfiler()
{
	std::ofstream myFile;
	myFile.open("example.csv");
	myFile << tempBuffer.rdbuf();
	myFile.close();
}

void GPUProfiler::Cleanup()
{
	if (beginFrameQuery != nullptr)
	{
		beginFrameQuery->Release();
	}

	if (endFrameQuery != nullptr)
	{
		endFrameQuery->Release();
	}

	if (disjoint != nullptr)
	{
		disjoint->Release();
	}

	if (renderStatisticsQuery != nullptr)
	{
		renderStatisticsQuery->Release();
	}
}

void GPUProfiler::Initialize(ID3D11Device* const aDevice)
{
	if (!hasBeenInitialized)
	{
		D3D11_QUERY_DESC qDesc;

		qDesc.MiscFlags = 0;

		// Create begin and end timestamp query
		qDesc.Query = D3D11_QUERY_TIMESTAMP;
		aDevice->CreateQuery(&qDesc, &beginFrameQuery);
		aDevice->CreateQuery(&qDesc, &endFrameQuery);

		// Create disjoint query
		qDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		aDevice->CreateQuery(&qDesc, &disjoint);

		qDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
		aDevice->CreateQuery(&qDesc, &renderStatisticsQuery);
		
		hasBeenInitialized = true;


		// If debug, define names for objects (useful for when graphics debugging)
	#if defined(_DEBUG)
		const char beginName[] = "QueryTimestampBeginFrame";
		beginFrameQuery->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(beginName) - 1, beginName);

		const char endName[] = "QueryTimestampEndFrame";
		endFrameQuery->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(endName) - 1, endName);

		const char disjointName[] = "QueryTimestampEndFrame";
		disjoint->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(disjointName) - 1, disjointName);
	#endif

		LOG(INFO) << "GPUprofiler: initialized";
		return;
	}

	else
	{
		LOG(WARNING) << "GPUprofiler: Trying to initialize the gpu profiler, but it already has been initialized";
	}
}


void GPUProfiler::BeginFrame(ID3D11DeviceContext* const aContext)
{
	hasStartedFrame = true;
	aContext->Begin(disjoint);
	aContext->End(beginFrameQuery);
	aContext->Begin(renderStatisticsQuery);
}


void GPUProfiler::EndFrame(ID3D11DeviceContext* const aContext)
{
	hasStartedFrame = false;
	aContext->End(renderStatisticsQuery);
	aContext->End(endFrameQuery);
	aContext->End(disjoint);
}


void GPUProfiler::SetStamp(ID3D11DeviceContext* const aContext)
{
	if (!hasStartedFrame)
	{
		LOG(WARNING) << "GPUProfiler: Can not set stamp if the frame has not started yet!";
	}
}


static int64_t currentFrame = 0;
void GPUProfiler::CollectData(ID3D11DeviceContext* const aContext)
{
	// Idle until our data is not in flight anymore
	while (aContext->GetData(disjoint, NULL, 0, 0) == S_FALSE)
	{
		Sleep(1);
	}

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tsDisjoint;
	
	aContext->GetData(disjoint, &tsDisjoint, sizeof(tsDisjoint), 0);

	if (tsDisjoint.Disjoint)
	{
		return;
	}

	uint64_t tsBeginFrame, tsEndFrame;
	aContext->GetData(beginFrameQuery, &tsBeginFrame, sizeof(uint64_t), 0);
	aContext->GetData(endFrameQuery, &tsEndFrame, sizeof(uint64_t), 0);

	float frameTime = float(tsEndFrame - tsBeginFrame) / float(tsDisjoint.Frequency) * 1000.0f;

	D3D11_QUERY_DATA_PIPELINE_STATISTICS tsPipeStats;
	aContext->GetData(renderStatisticsQuery, &tsPipeStats, sizeof(tsPipeStats), 0);


	tempBuffer << currentFrame << ";" << frameTime << "\n";
	currentFrame++;
}
