#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include <memory> 

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();

private:
	std::unique_ptr<D3DClass> m_Direct3D;
	std::unique_ptr<CameraClass> m_Camera;
	std::unique_ptr<ModelClass> m_Model;
	std::unique_ptr<ColorShaderClass> m_ColorShader;
};

#endif