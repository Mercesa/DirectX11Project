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
	mAmbientColor = XMFLOAT3(red, green, blue);
	return;
}


void d3dLightClass::SetDiffuseColor(float red, float green, float blue)
{
	mDiffuseColor = XMFLOAT3(red, green, blue);
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
	return mAmbientColor;
}


XMFLOAT3 d3dLightClass::GetDiffuseColor()
{
	return mDiffuseColor;
}


XMFLOAT3 d3dLightClass::GetPosition()
{
	return mPosition;
}

	
void d3dLightClass::GenerateViewMatrix()
{
	XMFLOAT3 up;

	// Create the view matrix from the three vectors.
	XMVECTOR upVector, positionVector, lookAtVector;

	// Up, position, look at
	upVector = XMLoadFloat3(&up);
	positionVector = XMLoadFloat3(&mPosition);
	lookAtVector = XMLoadFloat3(&mLookAt);
	
	// direction matrix
	XMMATRIX directionMatrix = XMMatrixRotationRollPitchYaw(mPosition.x, mPosition.y, mPosition.z);
	XMStoreFloat4x4(&mViewMatrix, directionMatrix);

	// Direction vector
	XMFLOAT3 dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR dirVec = XMLoadFloat3(&dir);
	dirVec = XMVector3Transform(dirVec, directionMatrix);
	XMStoreFloat3(&this->mDirectionVector, dirVec);

	return;
}


void d3dLightClass::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;


	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)D3DX_PI / 2.0f;
	screenAspect = 1.0f;

	//XMStoreFloat4x4(&mProjectionmatrix, XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth));
	XMStoreFloat4x4(&mProjectionmatrix, XMMatrixOrthographicLH(50.0f, 50.0f, -50.0f, 50.0f));

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