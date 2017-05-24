#include "inputclass.h"

#include "easylogging++.h"
// This class is taken from rastertek, possibly some changes are made but this is work not of my own design.
// http://www.rastertek.com/dx11tut13.html
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


bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;
	
	mMouseX = 0;
	mMouseY = 0;

	mMouseRelX = 0;
	mMouseRelY = 0;

	for (int i = 0; i < 256; ++i)
	{
		mKeys[i] = false;
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
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	//mMouseX += mMouseState.lX;
	//mMouseY	+= mMouseState.lY;
	//
	//mMouseRelX = mMouseState.lX;
	//mMouseRelY = mMouseState.lY;
	//
	//
	//// Ensure the mouse location doesn't exceed the screen width or height.
	//if (mMouseX < 0) { mMouseX = 0; }
	//if (mMouseY < 0) { mMouseY = 0; }
	//
	//if (mMouseX > mScreenWidth) { mMouseX = mScreenWidth; }
	//if (mMouseY > mScreenHeight) { mMouseY = mScreenHeight; }

	return;
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
