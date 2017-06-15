#include "d3dLightClass.h"


d3dLightClass::d3dLightClass()
{
}


d3dLightClass::d3dLightClass(const d3dLightClass& other)
{
}


d3dLightClass::~d3dLightClass()
{
}


void d3dLightClass::SetAmbientColor(float red, float green, float blue)
{
	m_ambientColor = XMFLOAT3(red, green, blue);
	return;
}


void d3dLightClass::SetDiffuseColor(float red, float green, float blue)
{
	m_diffuseColor = XMFLOAT3(red, green, blue);
	return;
}


void d3dLightClass::SetPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
}


void d3dLightClass::SetLookAt(float x, float y, float z)
{
	mLookAt.x = x;
	mLookAt.y = y;
	mLookAt.z = z;
}


XMFLOAT3 d3dLightClass::GetAmbientColor()
{
	return m_ambientColor;
}


XMFLOAT3 d3dLightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}


XMFLOAT3 d3dLightClass::GetPosition()
{
	return m_position;
}


void d3dLightClass::GenerateViewMatrix()
{
	XMFLOAT3 up;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Create the view matrix from the three vectors.
	
	XMVECTOR upVector, positionVector, lookAtVector;

	upVector = XMLoadFloat3(&up);
	positionVector = XMLoadFloat3(&mPosition);
	lookAtVector = XMLoadFloat3(&mLookAt);

	//D3DXMatrixLookAtLH(&m_viewMatrix, &m_position, &m_lookAt, &up);
	
	XMStoreFloat4x4(&mViewMatrix, XMMatrixLookAtLH(positionVector, lookAtVector, upVector));
	return;
}


void d3dLightClass::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;


	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)D3DX_PI / 2.0f;
	screenAspect = 1.0f;

	//XMStoreFloat4x4(&mProjectionmatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth));
	XMStoreFloat4x4(&mProjectionmatrix, XMMatrixOrthographicLH(32, 32, screenNear, screenDepth));
	return;
}


void d3dLightClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = XMLoadFloat4x4(&mViewMatrix);
}


void d3dLightClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = XMLoadFloat4x4(&mProjectionmatrix);
}