#pragma once

#include <d3d11.h>	
#include <directxmath.h>
using namespace DirectX;

#include <wrl.h>
#include <memory>

class d3dShaderManager;

class GraphicsEngine
{

	enum eSamplers { eFullWrapAntistropic };

public:
	GraphicsEngine();
	virtual ~GraphicsEngine();

	static GraphicsEngine& getInstance()
	{
		static GraphicsEngine gEngine;
		return gEngine;
	}

	// Initialize the graphics engine
	bool Initialize();

	// Create device
	bool CreateDevice();

	// Get device
	ID3D11Device* const GetDevice();

	// Get context
	ID3D11DeviceContext* const GetDeviceContext();

	void LoadShaders();


	
private:
	bool InitializeSamplers();
	
	// Prevent this class from being initialized
	GraphicsEngine(GraphicsEngine const&) = delete;
	void operator=(GraphicsEngine const&) = delete;


	Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;

	std::unique_ptr<d3dShaderManager> mpShaderManager;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m
};

