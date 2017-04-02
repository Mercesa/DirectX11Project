#include "modelclass.h"

#include "d3dVertexBuffer.h"
#include "ModelData.h"
ModelClass::ModelClass()
{
	mVertexBuffer = std::make_unique<d3dVertexBuffer>();
	mIndexBuffer = std::make_unique<d3dVertexBuffer>();
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* aDevice)
{
	bool result;


	// Initialize the vertex and index buffers.
	result = InitializeBuffers(aDevice);
	if(!result)
	{
		return false;
	}

	return true;
}

bool ModelClass::Initialize(ID3D11Device* aDevice, const MeshData& aMesh)
{
	bool result;


	// Initialize the vertex and index buffers.
	result = InitializeBuffers(aDevice, aMesh);
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


bool ModelClass::InitializeBuffers(ID3D11Device* aDevice)
{
	VertexType* vertices;
	unsigned long* indices;
	HRESULT result;
	

	// Set the number of vertices in the vertex array.
	int m_vertexCount = 3;

	// Set the number of indices in the index array.
	int m_indexCount = 3;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	vertices[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	vertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[2].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.


	if (!mVertexBuffer->Initialize(aDevice, (void*)vertices, sizeof(VertexType) * m_vertexCount, m_vertexCount, D3D11_BIND_VERTEX_BUFFER))
	{
		return false;
	}

	if (!mIndexBuffer->Initialize(aDevice, (void*)indices, sizeof(unsigned long) * m_indexCount, m_indexCount, D3D11_BIND_INDEX_BUFFER))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	//mIndexBuffer.release();
	//mVertexBuffer.release();
}

// Function for if we have any modeldata
bool ModelClass::InitializeBuffers(ID3D11Device* aDevice, const MeshData& aData)
{
	HRESULT result;
	
	
	size_t vertSize = aData.vertices.size();
	size_t indicesSize = aData.indices.size();

	if (!mVertexBuffer->Initialize(aDevice, (void*)aData.vertices.data(), sizeof(VertexType) * vertSize, vertSize, D3D11_BIND_VERTEX_BUFFER))
	{
		return false;
	}

	if (!mIndexBuffer->Initialize(aDevice, (void*)aData.indices.data(), sizeof(unsigned long) * indicesSize, indicesSize, D3D11_BIND_INDEX_BUFFER))
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
	stride = sizeof(VertexType); 
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