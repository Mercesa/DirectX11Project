#include "modelclass.h"

#include "d3dVertexBuffer.h"
#include "d3dTexture.h"
#include "d3dMaterial.h"
#include "GraphicsStructures.h"
#include "d3d11HelperFile.h"

ModelClass::ModelClass() 
	: mMaterial(nullptr)
{
	mVertexBuffer = std::make_unique<Buffer>();
	mIndexBuffer = std::make_unique<Buffer>();
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{
	mVertexBuffer->buffer->Release();
	mIndexBuffer->buffer->Release();
}


bool ModelClass::Initialize(ID3D11Device* const apDevice, const RawMeshData& acMesh)
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
	return mIndexBuffer->amountOfElements;
}

bool ModelClass::InitializeBuffers(ID3D11Device* aDevice, const RawMeshData& aData)
{	
	
	size_t vertSize = aData.vertices.size();
	size_t indicesSize = aData.indices.size();

	mVertexBuffer->buffer = CreateSimpleBuffer(aDevice, (void*)(aData.vertices.data()), sizeof(VertexData) * vertSize, vertSize, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT);
	mVertexBuffer->amountOfElements = indicesSize;
	
	mIndexBuffer->buffer = CreateSimpleBuffer(aDevice, (void*)(aData.indices.data()), sizeof(unsigned long) * indicesSize, indicesSize, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT);
	mIndexBuffer->amountOfElements = indicesSize;

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

	ID3D11Buffer* tBuffer = mVertexBuffer->buffer;

	deviceContext->IASetVertexBuffers(0, 1, &tBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(mIndexBuffer->buffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}