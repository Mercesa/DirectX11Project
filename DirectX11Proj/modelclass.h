#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <cstdint>

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

#include <memory>


class d3dVertexBuffer;
class d3dTexture;
struct RawMeshData;
struct Buffer;
class d3dMaterial;

class ModelClass
{
public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device* const apDevice, const RawMeshData& acMeshData);

	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int32_t GetIndexCount();

	std::unique_ptr<d3dMaterial> mMaterial;

private:
	bool InitializeBuffers(ID3D11Device* const apDevice, const RawMeshData& aMeshData);
	void RenderBuffers(ID3D11DeviceContext* const apDeviceContext);

private:
	int32_t tIndiceSize = 0;
	std::unique_ptr<Buffer> mVertexBuffer;
	std::unique_ptr<Buffer> mIndexBuffer;
};

#endif