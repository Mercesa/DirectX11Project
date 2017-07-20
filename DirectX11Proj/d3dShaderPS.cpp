#include "d3dShaderPS.h"

#include "ShaderHelperFunctions.h"
#include "d3dShaderManager.h"


d3dShaderPS::d3dShaderPS()
{
}


d3dShaderPS::~d3dShaderPS()
{
}

inline wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}


bool d3dShaderPS::LoadPixelShader(ID3D11Device* const apDevice, ShaderInfo& aInfo)
{
	HRESULT result;

	wchar_t* tWstring = convertCharArrayToLPCWSTR(aInfo.mFilePath);

	ID3D10Blob* tBlobRef = mpDataBlob.Get();
	LoadShaderWithErrorChecking(tWstring, (LPCSTR)aInfo.mEntryPoint, (LPCSTR)aInfo.mShaderProfile, tBlobRef);

	result = apDevice->CreatePixelShader(tBlobRef->GetBufferPointer(), tBlobRef->GetBufferSize(), NULL, &mpPixelShader);

	if (FAILED(result))
	{
		LOG(WARNING) << "Shader: " << aInfo.mFilePath << " failed to load";
		return false;
	}

	delete[] tWstring;

	return true;
}