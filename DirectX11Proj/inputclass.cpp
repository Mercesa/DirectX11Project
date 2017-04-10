#include "inputclass.h"

#include "easylogging++.h"
// This class is taken from rastertek, possibly some changes are made but this is work not of my own design.
// http://www.rastertek.com/dx11tut13.html
InputClass::InputClass()
{
	this->mpDirectInput = 0;
	this->mpKeyboard = 0;
	this->mpMouse = 0;
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;
	
	mMouseX = 0;
	mMouseY = 0;

	mMouseRelX = 0;
	mMouseRelY = 0;

	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mpDirectInput, NULL);
	if (FAILED(result))
	{
		LOG(ERROR) << "FAILED to create directInput";
		return false;
	}
	
	result = mpDirectInput->CreateDevice(GUID_SysKeyboard, &mpKeyboard, NULL);
	if (FAILED(result))
	{
		LOG(ERROR) << "FAILED to create keyboard";
		return false;
	}

	result = mpKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		LOG(ERROR) << "FAILED to set data format of keyboard";
		return false;
	}

	// Set the cooperative level of the keyboard to not share with other programs.
	result = mpKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}
	//	Once they keyboard is setup we then call Acquire to finally get access to the keyboard for use from this point forward.
	// Now acquire the keyboard.
	result = mpKeyboard->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the mouse.
	result = mpDirectInput->CreateDevice(GUID_SysMouse, &mpMouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = mpMouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	result = mpMouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}
	//Once the mouse is setup we acquire it so that we can begin using it.

	// Acquire the mouse.
	result = mpMouse->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void InputClass::Shutdown()
{
	// Release the mouse.
	if (mpMouse)
	{
		mpMouse->Unacquire();
		mpMouse->Release();
		mpMouse = 0;
	}

	// Release the keyboard.
	if (mpKeyboard)
	{
		mpKeyboard->Unacquire();
		mpKeyboard->Release();
		mpKeyboard = 0;
	}

	// Release the main interface to direct input.
	if (mpDirectInput)
	{
		mpDirectInput->Release();
		mpDirectInput = 0;
	}

	return;
}


bool InputClass::Frame()
{
	bool result;

	// Read the current state of the keyboard.
	result = ReadKeyboard();
	if (!result)
	{
		return false;
	}

	// Read the current state of the mouse.
	result = ReadMouse();
	if (!result)
	{
		return false;
	}

	// Process the changes in the mouse and keyboard.
	ProcessInput();

	return true;
}


bool InputClass::ReadKeyboard()
{
	HRESULT result;


	// Read the keyboard device.
	result = mpKeyboard->GetDeviceState(sizeof(mKeyboardState), (LPVOID)&mKeyboardState);
	if (FAILED(result))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			mpKeyboard->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}


bool InputClass::ReadMouse()
{
	HRESULT result;


	// Read the mouse device.
	result = mpMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mMouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			mpMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}


void InputClass::ProcessInput()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	mMouseX += mMouseState.lX;
	mMouseY	+= mMouseState.lY;

	mMouseRelX = mMouseState.lX;
	mMouseRelY = mMouseState.lY;


	// Ensure the mouse location doesn't exceed the screen width or height.
	if (mMouseX < 0) { mMouseX = 0; }
	if (mMouseY < 0) { mMouseY = 0; }

	if (mMouseX > mScreenWidth) { mMouseX = mScreenWidth; }
	if (mMouseY > mScreenHeight) { mMouseY = mScreenHeight; }

	return;
}


bool InputClass::IsEscapePressed()
{
	// Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
	if (mKeyboardState[DIK_ESCAPE] & 0x80)
	{
		return true;
	}

	return false;
}


void InputClass::GetMouseLocation(int& mouseX, int& mouseY)
{
	mouseX = mMouseX;
	mouseY = mMouseY;
}

void InputClass::GetMouseRelLocation(int& aX, int& aY)
{
	aX = mMouseRelX;
	aY = mMouseRelY;
}
