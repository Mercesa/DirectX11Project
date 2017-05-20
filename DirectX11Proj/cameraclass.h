
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


#include <directxmath.h>
using namespace DirectX;


class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void UpdateViewMatrix();
	void GetViewMatrix(XMMATRIX&);

	float m_positionX, m_positionY, m_positionZ;
	
	XMFLOAT3 up, position, lookAt;

private:
	float m_rotationX, m_rotationY, m_rotationZ;
	XMFLOAT4X4 mViewMatrix;
	XMFLOAT4X4 mProjectionMatrix;
};

#endif