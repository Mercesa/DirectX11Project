#include "d3dShaderVS.h"

#include "ShaderHelperFunctions.h"
#include "d3dShaderManager.h"

d3dShaderVS::d3dShaderVS()
{
}


d3dShaderVS::~d3dShaderVS()
{
}

inline wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}


bool d3dShaderVS::LoadVertexShader(ID3D11Device* const apDevice, ShaderInfo& aInfo)
{

	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;


	polygonLayout[3].SemanticName = "BITANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "TANGENT";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;


	HRESULT result;


	wchar_t* tWstring = convertCharArrayToLPCWSTR(aInfo.mFilePath);

	ID3D10Blob* tBlobRef = mpDataBlob.Get();
	LoadShaderWithErrorChecking(tWstring, (LPCSTR)aInfo.mEntryPoint, (LPCSTR)aInfo.mShaderProfile, tBlobRef);

	result = apDevice->CreateVertexShader(tBlobRef->GetBufferPointer(), tBlobRef->GetBufferSize(), NULL, this->mpVertexShader.GetAddressOf());
	
	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " failed to load";
		return false;
	}


	// Get a count of the elements in the layout.
	uint32_t tNumElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = apDevice->CreateInputLayout(polygonLayout, tNumElements, tBlobRef->GetBufferPointer(),
		tBlobRef->GetBufferSize(), &mpLayout);
	
	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " input layout failed to create";
		return false;
	}


	delete[] tWstring;

	return true;

}