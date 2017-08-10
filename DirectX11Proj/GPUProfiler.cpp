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
	WriteStatisticsToFile();
}

void GPUProfiler::Shutdown()
{
	// Release all resources we have
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

	// Release all timestamps
	for (auto& e : queryStamps)
	{
		if (e->beginMark != nullptr)
		{
			e->beginMark->Release();
		}

		if (e->endMark != nullptr)
		{
			e->endMark->Release();
		}
	}
}


void GPUProfiler::WriteStatisticsToFile()
{
	std::ofstream myFile;
	myFile.open("example.csv");

	// Header of CSV
	myFile << "Frame" << ";" << "ms on GPU" << ";";

	for (auto &e : queryStamps)
	{
		// Add every query stamp their name
		myFile << e->name << ";";
	}

	// next line for the numbers
	myFile << "\n";


	myFile << tempBuffer.rdbuf();
	myFile.close();
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

		const char disjointName[] = "QueryTimeDisjoint";
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
	// Reset all stamps to their original state
	for (auto &e : queryStamps)
	{
		e->hasStarted = false;
		e->hasFinished = false;
	}

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

	// Reset all stamps
}


// When we need to create a query stamp
QueryStamp* GPUProfiler::CreateQueryStamp(ID3D11DeviceContext* const aContext, ID3D11Device* const aDevice, std::string aName)
{
	// Check if a query stamp with the same name already exists (and check if its still available)
	for (auto &e : queryStamps)
	{
		if (e->name == aName)
		{			
			return e.get();
		}
	}

	LOG(INFO) << "GPUProfiler: Stamp with name " << aName << " does not exist yet, creating new stamp";
	std::unique_ptr<QueryStamp> tStamp = std::make_unique<QueryStamp>();
	tStamp->name = aName;

	D3D11_QUERY_DESC qDesc;

	qDesc.MiscFlags = 0;

	// Create begin and end timestamp query
	qDesc.Query = D3D11_QUERY_TIMESTAMP;
	
	aDevice->CreateQuery(&qDesc, &tStamp->beginMark);
	aDevice->CreateQuery(&qDesc, &tStamp->endMark);
	
	QueryStamp* retStamp = tStamp.get();
	queryStamps.push_back(std::move(tStamp));
	
	return retStamp;
}

void GPUProfiler::SetStamp(ID3D11DeviceContext* const aContext, ID3D11Device* const aDevice, std::string aName)
{
	// Frame needs to have started before time stamps can be marked
	if (!hasStartedFrame)
	{
		LOG(WARNING) << "GPUProfiler: Can not set stamp if the frame has not started yet!";
		return;
	}


	else
	{
		// Create a stamp ( or get an existing one )
		QueryStamp* qS = CreateQueryStamp(aContext, aDevice, aName);
		
		// If stamp does not have a starting mark
		if (!qS->hasStarted)
		{
			// Mark it
			aContext->End(qS->beginMark);
			qS->hasStarted = true;
			return;
		}
		
		// If stamp HAS started and not been finished, mark it again
		else
		{
			if (qS->hasFinished == false)
			{
				aContext->End(qS->endMark);
				qS->hasFinished = true;
			}

			// We use stamps to mark a start and end point, the user tried to use a stamp three times in one frame, which is not supported
			else
			{
				LOG(WARNING) << "GPUProfiler: User attempted to mark a query more than two times";
			}
		}
	}
}


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

	tempBuffer << currentFrame << ";" << frameTime << ";";
	// Go through all the stamps and put their frame-time down
	for (int i = 0; i < queryStamps.size(); ++i)
	{
		// Only stamps that have been finished are able to have a difference in time
		// Since unfinished stamps means that a start point has been set but a begin point never has been set
		if (queryStamps[i]->hasFinished == true)
		{
			uint64_t beginStamp, endStamp;
			aContext->GetData(queryStamps[i]->beginMark, &beginStamp, sizeof(uint64_t), 0);
			aContext->GetData(queryStamps[i]->endMark, &endStamp, sizeof(uint64_t), 0);

			float frameStamps = float(endStamp - beginStamp) / float(tsDisjoint.Frequency) * 1000.0f;

			std::cout << "Frametime of " << queryStamps[i]->name << " took: " << frameStamps << "\n";

			tempBuffer << frameStamps << ";";
		}
	}

	tempBuffer << "\n";

	currentFrame++;
}
