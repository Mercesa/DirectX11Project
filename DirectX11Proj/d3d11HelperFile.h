#pragma once

#include <glm/common.hpp>
#include <glm/gtx/common.hpp>

#include <windowsx.h>
#include <wrl.h>

#include <fcntl.h>
#include <d3d11_1.h>
#include "easylogging++.h"

#include <cassert>
#include "GraphicsStructures.h"
#include "GenericMathValueStructs.h"

// These ID structs are special due to their explicit constructors
// This prevents issues such as using a texture ID for loading a model ID
struct TexID
{
	explicit TexID(uint32_t id) : id(id) {}
	explicit TexID() : id(0) {}

	const uint32_t GetID() const { return id; }

private:
	uint32_t id;
};

struct MatID
{
	explicit MatID(uint32_t id) : id(id) {}
	explicit MatID() : id(0) {}

	const uint32_t GetID() const { return id; }

private:
	uint32_t id;
};

struct ModelID
{
	explicit ModelID(uint32_t id) : id(id) {}
	ModelID() : id(0) {}

	const uint32_t GetID() const { return id; }
private:
	uint32_t id;
};

// Texture is a collection of views and a texture
struct Texture
{
	ID3D11Texture2D* texture = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;
};

struct Buffer
{
	ID3D11Buffer* buffer = nullptr;
	// The amount of elements in the buffer
	size_t amountOfElements = 0;
};

struct VertexShader
{
	ID3D11VertexShader* shader;
	ID3D11InputLayout* inputLayout;
};

struct PixelShader
{
	ID3D11PixelShader* shader;
};

struct ComputeShader
{
	ID3D11ComputeShader* shader;
};

struct Material
{
	Material() : mpDiffuse(0), mpNormal(0), mpSpecular(0) {}
	TexID mpDiffuse = TexID();
	TexID mpSpecular = TexID();
	TexID mpNormal = TexID();
};

// A model is nothing more than something with a vertexbuffer and index buffer
struct Model
{
	Buffer* vertexBuffer;
	Buffer* indexBuffer;
	Material* material;

	// x y z are for the center of the sphere, the w is for the radius
	VEC4f sphereCollider;
};

struct ShadowMap 
{
	D3D11_VIEWPORT viewport;

	Texture* resource;

	uint32_t width = 0; 
	uint32_t height = 0;
};

#include "FrustumG.h"
struct CameraData
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 position;
	const FrustumG* frustumPtr;
	float nearZ, farZ;
	float fovY;
	float aspect;
};

struct LightData
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 dirVector;

	glm::vec3 ambientCol;
	glm::vec3 diffuseCol;
	glm::vec3 specularCol;
	glm::vec3 position;
};


struct FrameData
{
	float totalTime;
	float deltaTime;
	float framerate;
};


static void ReleaseVertexShader(VertexShader* aVShader)
{
	assert(aVShader != nullptr);
	if (aVShader->shader != nullptr)
	{
		aVShader->shader->Release();
	}

	if (aVShader->inputLayout != nullptr)
	{
		aVShader->inputLayout->Release();
	}
}

static void ReleasePixelShader(PixelShader* aPShader)
{
	assert(aPShader != nullptr);
	if (aPShader->shader != nullptr)
	{
		aPShader->shader->Release();
	}
}

static void ReleaseComputeShader(ComputeShader* aCShader)
{
	assert(aCShader != nullptr);
	if (aCShader->shader != nullptr)
	{
		aCShader->shader->Release();
	}
}

static void ReleaseTexture(Texture* aTexture)
{
	assert(aTexture != nullptr);
	if (aTexture->texture != nullptr)
	{
		aTexture->texture->Release();
	}

	if (aTexture->dsv != nullptr)
	{
		aTexture->dsv->Release();
	}

	if (aTexture->rtv != nullptr)
	{
		aTexture->rtv->Release();
	}

	if (aTexture->srv != nullptr)
	{
		aTexture->srv->Release();
	}

	if (aTexture->uav != nullptr)
	{
		aTexture->uav->Release();
	}
}

static void ReleaseModel(Model* aModel)
{
	assert(aModel != nullptr);

	if (aModel->vertexBuffer != nullptr)
	{
		if (aModel->vertexBuffer->buffer != nullptr)
		{
			aModel->vertexBuffer->buffer->Release();
		}
		
		delete aModel->vertexBuffer;
	}

	if (aModel->indexBuffer != nullptr)
	{
		if (aModel->indexBuffer->buffer != nullptr)
		{
			aModel->indexBuffer->buffer->Release();
		}
		delete aModel->indexBuffer;
	}
}


/*
*******************************
* VERTEX BUFFER LAYOUT FUNCTIONS
*******************************
*/

// Argument will be changed internally
static void CreateVertexbufferLayoutDefault(D3D11_INPUT_ELEMENT_DESC* const aLayout)
	{
		aLayout[0].SemanticName = "POSITION";
		aLayout[0].SemanticIndex = 0;
		aLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		aLayout[0].InputSlot = 0;
		aLayout[0].AlignedByteOffset = 0;
		aLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		aLayout[0].InstanceDataStepRate = 0;
		
		aLayout[1].SemanticName = "TEXCOORD";
		aLayout[1].SemanticIndex = 0;
		aLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		aLayout[1].InputSlot = 0;
		aLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		aLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		aLayout[1].InstanceDataStepRate = 0;
		
		aLayout[2].SemanticName = "NORMAL";
		aLayout[2].SemanticIndex = 0;
		aLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		aLayout[2].InputSlot = 0;
		aLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		aLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		aLayout[2].InstanceDataStepRate = 0;
		
		aLayout[3].SemanticName = "BITANGENT";
		aLayout[3].SemanticIndex = 0;
		aLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		aLayout[3].InputSlot = 0;
		aLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		aLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		aLayout[3].InstanceDataStepRate = 0;
		
		aLayout[4].SemanticName = "TANGENT";
		aLayout[4].SemanticIndex = 0;
		aLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		aLayout[4].InputSlot = 0;
		aLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		aLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		aLayout[4].InstanceDataStepRate = 0;
	}

/*
*******************************
* RASTERIZER CREATION FUNCTIONS
*******************************
*/
static ID3D11RasterizerState* CreateRSDefault(ID3D11Device* const aDevice)
{
	ID3D11RasterizerState* tRaster = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = aDevice->CreateRasterizerState(&rasterDesc, &tRaster);
	
	if (FAILED(result))
	{
		LOG(INFO) << "CreateDefaultRasterizerState failed";
		return nullptr;
	}

	return tRaster;
}

static ID3D11RasterizerState* CreateRSCullNone(ID3D11Device* const aDevice)
{
	ID3D11RasterizerState* tRaster = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = aDevice->CreateRasterizerState(&rasterDesc, &tRaster);

	if (FAILED(result))
	{
		LOG(INFO) << "CreateDefaultRasterizerState failed";
		return nullptr;
	}

	return tRaster;
}

static ID3D11RasterizerState* CreateRSNoCull(ID3D11Device* const aDevice)
{
	ID3D11RasterizerState* tRaster = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;


	result = aDevice->CreateRasterizerState(&rasterDesc, &tRaster);

	if (FAILED(result))
	{
		LOG(INFO) << "CreateRSNoCull failed";
		return nullptr;
	}

	return tRaster;
}



static ID3D11RasterizerState* CreateRSWireFrame(ID3D11Device* const aDevice)
{
	ID3D11RasterizerState* tRaster = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc;
	HRESULT result;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;


	result = aDevice->CreateRasterizerState(&rasterDesc, &tRaster);

	if (FAILED(result))
	{
		LOG(ERROR) << "CreateDefaultRasterizerState failed";
		return nullptr;
	}

	return tRaster;
}


/*
***********************************
**SAMPLER STATE DEFAULT FUNCTIONS**
***********************************
*/

static ID3D11SamplerState* CreateSamplerAnisotropicWrap(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerAnisotropicWrap failed";
		return nullptr;
	}

	return sampler;
}

static ID3D11SamplerState* CreateSamplerPointClamp(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerPointClamp failed";
		return nullptr;
	}

	return sampler;
}

static ID3D11SamplerState* CreateSamplerPointWrap(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerPointClamp failed";
		return nullptr;
	}

	return sampler;
}


static ID3D11SamplerState* CreateSamplerLinearClamp(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.	
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerPointClamp failed";
		return nullptr;
	}

	return sampler;
}

static ID3D11SamplerState* CreateSamplerLinearWrap(ID3D11Device* const aDevice)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ID3D11SamplerState* sampler;

	// Create a texture sampler state description.	
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = aDevice->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSamplerPointClamp failed";
		return nullptr;
	}

	return sampler;
}


/*
***************************
**DEPTH STENCIL STATE FUNCTIONS**
***************************
*/
static ID3D11DepthStencilState* CreateDepthStateDefault(ID3D11Device* const aDevice)
{
	ID3D11DepthStencilState* tDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	HRESULT result;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = aDevice->CreateDepthStencilState(&depthStencilDesc, &tDepthStencilState);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth-stencil state";
		return nullptr;
	}

	return tDepthStencilState;
}

static ID3D11DepthStencilState* CreateDepthStateDeferred(ID3D11Device* const aDevice)
{
	ID3D11DepthStencilState* tDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	HRESULT result;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = aDevice->CreateDepthStencilState(&depthStencilDesc, &tDepthStencilState);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth-stencil state";
		return nullptr;
	}

	return tDepthStencilState;
}

static ID3D11DepthStencilState* CreateDepthStateLessEqual(ID3D11Device* const aDevice)
{
	ID3D11DepthStencilState* tDepthStencilState;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	HRESULT result;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = aDevice->CreateDepthStencilState(&depthStencilDesc, &tDepthStencilState);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth-stencil state";
		return nullptr;
	}

	return tDepthStencilState;
}


/*
********************************
**RENDER TARGET VIEW FUNCTIONS**
********************************
*/

static ID3D11Texture2D* CreateSimpleTexture2D(ID3D11Device* const aDevice, uint32_t aWidth, uint32_t aHeight, DXGI_FORMAT aFormat, UINT aBindFlags)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	HRESULT result;
	ID3D11Texture2D* texture;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = aWidth;
	depthBufferDesc.Height = aHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = aFormat;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = aBindFlags;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = aDevice->CreateTexture2D(&depthBufferDesc, NULL, &texture);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSimpleTexture2D failed";
		return nullptr;
	}
	return texture;
}

static ID3D11RenderTargetView* CreateSimpleRenderTargetView(ID3D11Device* const aDevice, ID3D11Texture2D* aTexture, DXGI_FORMAT aFormat)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ID3D11RenderTargetView* rtv = nullptr;
	HRESULT result;

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = aFormat;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = aDevice->CreateRenderTargetView(aTexture, &renderTargetViewDesc, &rtv);
	if (FAILED(result))
	{
		return nullptr;
	}

	return rtv;
}


static ID3D11DepthStencilView* CreateSimpleDepthstencilView(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture, DXGI_FORMAT aFormat)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ID3D11DepthStencilView* dsv;
	HRESULT result;

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = aFormat;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = aDevice->CreateDepthStencilView(aTexture, &depthStencilViewDesc, &dsv);

	if (FAILED(result))
	{
		LOG(ERROR) << "Failed to create depth stencil view";
		return nullptr;
	}

	return dsv;
}

static ID3D11ShaderResourceView* CreateSimpleShaderResourceView(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture, DXGI_FORMAT aFormat)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ID3D11ShaderResourceView* srv;
	HRESULT result;
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = aFormat;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view.
	result = aDevice->CreateShaderResourceView(aTexture, &shaderResourceViewDesc, &srv);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSimpleShaderResourceView failed";
		return nullptr;
	}

	return srv;
}

static ID3D11UnorderedAccessView* CreateSimpleUnorderedAccessView(ID3D11Device* const aDevice, ID3D11Texture2D* const aTexture, DXGI_FORMAT aFormat)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ID3D11UnorderedAccessView* uav;
	HRESULT result;
	// Setup the description of the shader resource view.
	uavDesc.Format = aFormat;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	// Create the shader resource view.
	result = aDevice->CreateUnorderedAccessView(aTexture, &uavDesc, &uav);
	if (FAILED(result))
	{
		LOG(ERROR) << "CreateSimpleShaderResourceView failed";
		return nullptr;
	}

	return uav;
}


static ID3D11RenderTargetView* CreateRenderTargetViewFromSwapchain(ID3D11Device* const aDevice, IDXGISwapChain* const aSwapChain)
{
	ID3D11Texture2D* backBufferPtr;
	ID3D11RenderTargetView* rtv;

	HRESULT result = aSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to get back buffer ptr from swapchain";
		return nullptr;
	}

	// Create the render target view with the back buffer pointer.
	result = aDevice->CreateRenderTargetView(backBufferPtr, NULL, &rtv);
	if (FAILED(result))
	{
		LOG(ERROR) << "failed to create render target view";
		return nullptr;
	}

	// Release the backbuffer ptr
	backBufferPtr->Release();
	backBufferPtr = 0;

	return rtv;
}


// Format conversion functions
static DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthformat)
{
	DXGI_FORMAT resformat;
	switch (depthformat)
	{
	case DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
		resformat = DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
		resformat = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
		resformat = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		resformat = DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS;
		break;

	default:
		LOG(ERROR) << "GetDepthResourceFormat: Unknown format entered";
		break;
	}

	return resformat;
}

static DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthformat)
{
	DXGI_FORMAT srvformat;
	switch (depthformat)
	{
	case DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
		srvformat = DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
		srvformat = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
		srvformat = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		srvformat = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	default:
		LOG(ERROR) << "GetDepthSRVFormat: Unknown format entered";
		break;
	}
	return srvformat;
}


// Create simple buffer(generally used for vertex/index buffer)
static ID3D11Buffer* CreateSimpleBuffer(ID3D11Device* const aDevice, void* aDataPtr, size_t aSizeInBytes, uint32_t amountOfElements, uint32_t aBindFlags, uint32_t aUsage)
{
	if (aSizeInBytes == 0)
	{
		LOG(ERROR) << "CreateSimpleBuffer failed, size of buffer to be created is 0";
		return nullptr;
	}
	HRESULT result;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	ID3D11Buffer* buffer;


	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE(aUsage);
	vertexBufferDesc.ByteWidth = (UINT)aSizeInBytes;
	vertexBufferDesc.BindFlags = (D3D11_BIND_FLAG)aBindFlags;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = aDataPtr;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = aDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &buffer);

	if (FAILED(result))
	{
		std::cout << "CreateSimpleBuffer failed" << std::endl;
		return false;
	}

	//mAmountOfElements = aAmountOfElements;

	return buffer;
}

static void UpdateBufferWithData(ID3D11DeviceContext* const aContext, ID3D11Buffer* const aBuffer, void* data)
{
	D3D11_MAPPED_SUBRESOURCE res;
	HRESULT result = aContext->Map(aBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	if (!FAILED(result))
	{
		memcpy(res.pData, data, res.DepthPitch);

		aContext->Unmap(aBuffer, 0);
	}

	else
	{
		LOG(ERROR) << "UpdateBufferWithData failed";
	}
}

static Model* CreateSimpleModelFromRawData(ID3D11Device* aDevice, const RawMeshData& aData)
{
	Model* model = new Model();
	model->vertexBuffer = new Buffer();
	model->indexBuffer = new Buffer();

	size_t vertSize = aData.vertices.size();
	size_t indicesSize = aData.indices.size();

	model->vertexBuffer->buffer = CreateSimpleBuffer(aDevice, (void*)(aData.vertices.data()), sizeof(VertexData) * vertSize, (uint32_t)vertSize, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT);
	model->vertexBuffer->amountOfElements = indicesSize;

	model->indexBuffer->buffer = CreateSimpleBuffer(aDevice, (void*)(aData.indices.data()), sizeof(unsigned long) * indicesSize, (uint32_t)indicesSize, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT);
	model->indexBuffer->amountOfElements = indicesSize;

	return model;
}



//static ID3D11RenderTargetView* CreateTexture2DRTVDefault(ID3D11Device* const aDevice, uint32_t aWidth, uint32_t aHeight)
//{
//	HRESULT result;
//	D3D11_TEXTURE2D_DESC textureDesc;
//	ID3D11Texture2D* tTexture = nullptr;
//
//	// Initialize the render target texture description.
//	ZeroMemory(&textureDesc, sizeof(textureDesc));
//
//	// Setup the render target texture description.
//	textureDesc.Width = aWidth;
//	textureDesc.Height = aHeight;
//	textureDesc.MipLevels = 1;
//	textureDesc.ArraySize = 1;
//	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
//	textureDesc.SampleDesc.Count = 1;
//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
//	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//	textureDesc.CPUAccessFlags = 0;
//	textureDesc.MiscFlags = 0;
//
//	// Create the render target texture.
//	result = aDevice->CreateTexture2D(&textureDesc, NULL, &tTexture);
//	if (FAILED(result))
//	{
//		LOG(ERROR) << "CreateTexture2DRTVDefault failed";
//		return nullptr;
//	}
//
//	return tTexture;
//}

