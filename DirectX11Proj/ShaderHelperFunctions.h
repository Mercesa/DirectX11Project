#pragma once

#include <d3d11.h>	
#include <D3DX11async.h>

#include <fstream>

static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

// Contains reference to a pointer, its a bit hacky but D3DX11Compile from file requires a reference
static bool LoadShaderWithErrorChecking(LPCWSTR aFileName, LPCSTR aFunctionName, LPCSTR shaderProfile, ID3D10Blob*& aContainer, HWND hwnd)
{
	HRESULT result;


	ID3D10Blob* errorMessage;
	errorMessage = 0;

	result = D3DX11CompileFromFile(aFileName, NULL, NULL, aFunctionName, shaderProfile, D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&aContainer, &errorMessage, NULL);

	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, (WCHAR*)aFileName);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, aFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}
	return true;
}