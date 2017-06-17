
#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_



#include <d3dx10math.h>
#include <DirectXMath.h>

using namespace DirectX;

class d3dLightClass
{
public:
	d3dLightClass();
	d3dLightClass(const d3dLightClass&);
	~d3dLightClass();

	void SetAmbientColor(float, float, float);
	void SetDiffuseColor(float, float, float);
	void SetPosition(float, float, float);
	void SetLookAt(float, float, float);

	XMFLOAT3 GetAmbientColor();
	XMFLOAT3 GetDiffuseColor();
	XMFLOAT3 GetPosition();

	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float, float);

	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);

private:
	XMFLOAT3 m_ambientColor;
	XMFLOAT3 m_diffuseColor;
	XMFLOAT3 m_position;

	XMFLOAT3 mPosition;
	XMFLOAT3 mLookAt;


	XMFLOAT4X4	mViewMatrix;
	XMFLOAT4X4  mProjectionmatrix;
	XMFLOAT4X4  mOrthoMatrix;
};

#endif