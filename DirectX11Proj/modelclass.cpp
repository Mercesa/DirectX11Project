#include "modelclass.h"

#include "d3dVertexBuffer.h"
#include "d3dTexture.h"
#include "d3dMaterial.h"
#include "ModelData.h"


ModelClass::ModelClass() 
	: mMaterial(nullptr)
{
	mVertexBuffer = std::make_unique<d3dVertexBuffer>();
	mIndexBuffer = std::make_unique<d3dVertexBuffer>();
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{}


bool ModelClass::Initialize(ID3D11Device* const apDevice, const MeshData& acMesh)
{
	bool result;

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(apDevice, acMesh);
	if (!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


int ModelClass::GetIndexCount()
{
	return mIndexBuffer->GetAmountOfElements();
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	//mIndexBuffer.release();
	//mVertexBuffer.release();
}


bool ModelClass::InitializeBuffers(ID3D11Device* aDevice, const MeshData& aData)
{	
	
	size_t vertSize = aData.vertices.size();
	size_t indicesSize = aData.indices.size();

	if (!mVertexBuffer->Initialize(aDevice, (void*)(aData.vertices.data()), sizeof(VertexData) * vertSize, vertSize, D3D11_BIND_VERTEX_BUFFER))
	{
		return false;
	}

	if (!mIndexBuffer->Initialize(aDevice, (void*)(aData.indices.data()), sizeof(unsigned long) * indicesSize, indicesSize, D3D11_BIND_INDEX_BUFFER))
	{
		return false;
	}

	return true;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexData);
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.

	ID3D11Buffer* tBuffer = mVertexBuffer->GetBuffer();

	deviceContext->IASetVertexBuffers(0, 1, &tBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(mIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}