#pragma once

#include <d3d11.h>	
#include <directxmath.h>
using namespace DirectX;

#include <wrl.h>
#include <memory>

class d3dShaderManager;
//class d3dDXGIManager;
//class d3dSwapchain;

class GraphicsEngine
{

	//enum eSamplers { eFullWrapAntistropic };

public:
	GraphicsEngine();
	virtual ~GraphicsEngine();

	static GraphicsEngine& getInstance()
	{
		static GraphicsEngine gEngine;
		return gEngine;
	}

	// Initialize the graphics engine
	bool Initialize(int aScreenWidth, int aScreenHeight, HWND hwnd);

	// Create device
	bool CreateDevice();

	// Get device
	ID3D11Device* const GetDevice();

	// Get context
	ID3D11DeviceContext* const GetDeviceContext();

	//void LoadShaders();
	ID3D11SamplerState* const GetAnisotropicWrapSampler();


	void SetProjectionMatrix(XMFLOAT4X4 aProjMatrix);
	void SetViewMatrix(XMFLOAT4X4 aViewMatrix);

	XMFLOAT4X4 GetProjectionMatrix();
	XMFLOAT4X4 GetViewMatrix();

	d3dShaderManager* const GetShaderManager();

	//d3dSwapchain* const GetSwapchain();

	void EndScene();

private:
	bool InitializeSamplers();
	
	// Prevent this class from being initialized
	GraphicsEngine(GraphicsEngine const&) = delete;
	void operator=(GraphicsEngine const&) = delete;


	Microsoft::WRL::ComPtr<ID3D11Device> mpDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mpDeviceContext;

	//std::unique_ptr<d3dSwapchain> mpSwapChain;

	std::unique_ptr<d3dShaderManager> mpShaderManager;
	//std::unique_ptr<d3dDXGIManager> mpDXGIManager;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> mpAnisotropicWrapSampler;

	XMFLOAT4X4 mProjectionMatrix;
	XMFLOAT4X4 mViewMatrix;

	bool hasBeenInitialized;
};

