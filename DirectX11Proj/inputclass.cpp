#include "inputclass.h"

#include "easylogging++.h"


InputClass::InputClass()
{
	//this->mpDirectInput = 0;
	//this->mpKeyboard = 0;
	//this->mpMouse = 0;
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


bool InputClass::Initialize()
{
	HRESULT result;
	
	mMouseX = 0;
	mMouseY = 0;

	mMouseRelX = 0;
	mMouseRelY = 0;

	for (int i = 0; i < 256; ++i)
	{
		mKeys[i] = false;
		mKeysC[i] = false;
	}

	return true;
}


void InputClass::Shutdown()
{

}


bool InputClass::Frame()
{
	// Process the changes in the mouse and keyboard.
	ProcessInput();
	return true;
}

void InputClass::ProcessInput()
{
	mMouseRelX = 0;
	mMouseRelY = 0;
	for (int i = 0; i < 256; ++i)
	{
		mKeys[i] = false;
		rMouseDown = false;
	}
}


bool InputClass::IsEscapePressed()
{
	// Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
	if (mKeys[VK_ESCAPE])
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

void InputClass::GetMouseRelativeLocation(int& aX, int& aY)
{
	aX = mMouseRelX;
	aY = mMouseRelY;
}
