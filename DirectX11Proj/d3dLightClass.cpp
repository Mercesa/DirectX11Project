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


void d3dLightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}


void d3dLightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}


void d3dLightClass::SetPosition(float x, float y, float z)
{
	m_position = D3DXVECTOR3(x, y, z);
	return;
}


void d3dLightClass::SetLookAt(float x, float y, float z)
{
	m_lookAt.x = x;
	m_lookAt.y = y;
	m_lookAt.z = z;
	return;
}


D3DXVECTOR4 d3dLightClass::GetAmbientColor()
{
	return m_ambientColor;
}


D3DXVECTOR4 d3dLightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}


D3DXVECTOR3 d3dLightClass::GetPosition()
{
	return m_position;
}


void d3dLightClass::GenerateViewMatrix()
{
	D3DXVECTOR3 up;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Create the view matrix from the three vectors.
	D3DXMatrixLookAtLH(&m_viewMatrix, &m_position, &m_lookAt, &up);
	
	return;
}


void d3dLightClass::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;


	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)D3DX_PI / 2.0f;
	screenAspect = 1.0f;

	// Create the projection matrix for the light.
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

	return;
}


void d3dLightClass::GetViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}


void d3dLightClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}