#include <vld.h>

#include "systemclass.h"

#include <memory>

// Include and initialize easy logging here, while its part of the System, NO file will ever include main.
// And its nice to have a central place for the logging library
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	std::unique_ptr<SystemClass> System;
	bool result;
	
	// Create the system object.
	System = std::make_unique<SystemClass>();
	if(!System)
	{
		LOG(FATAL) << "System failed on creation";
		return 0;
	}


	// Initialize and run the system object.
	result = System->Initialize();
	if(result)
	{
		System->Run();
	}

	else
	{
		LOG(FATAL) << "System failed on initialization";
	}

	// Shutdown and release the system object.
	System->Shutdown();

	return 0;
}