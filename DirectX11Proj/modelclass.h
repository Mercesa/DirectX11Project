#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

#include <memory>

class d3dVertexBuffer;

struct MeshData;

class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*);
	bool Initialize(ID3D11Device*, const MeshData&);

	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	bool InitializeBuffers(ID3D11Device*, const MeshData&);

	// Initialize buffers using previously loaded data


	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	std::unique_ptr<d3dVertexBuffer> mVertexBuffer;
	std::unique_ptr<d3dVertexBuffer> mIndexBuffer;

};

#endif