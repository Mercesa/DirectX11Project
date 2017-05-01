#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include <memory> 
#include <vector>

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"
#include "textureshaderclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 2.0f;

class IScene;

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(IScene *const);

private:
	bool Render(IScene* const);

private:
	std::unique_ptr<D3DClass> mpDirect3D;
	std::unique_ptr<ColorShaderClass> mpColorShader;
	std::unique_ptr<textureshaderclass> mTextureShader;
};
	
#endif