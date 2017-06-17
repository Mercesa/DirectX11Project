#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>


class InputClass
{
public:
	InputClass();
	InputClass(const InputClass&);
	~InputClass();

	bool Initialize();
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	void GetMouseLocation(int&, int&);
	void GetMouseRelativeLocation(int& aX, int& aY);

	void MouseMove(int aX, int aY)
	{
		mMouseRelX = aX - mMouseX;
		mMouseRelY = aY - mMouseY;
		mMouseX = aX;
		mMouseY = aY;
	}

	void KeyDown(unsigned int input)
	{
		// If a key is pressed then save that state in the key array.
		mKeys[input] = true;
		return;
	}


	void KeyUp(unsigned int input)
	{
		// If a key is released then clear that state in the key array.
		mKeys[input] = false;
		return;
	}


	bool IsKeyDown(unsigned int key)
	{
		// Return what state the key is in (pressed/not pressed).
		return mKeys[key];
	}


private:
	void ProcessInput();

	DIMOUSESTATE mMouseState;

	int mMouseX, mMouseY;
	int mMouseRelX, mMouseRelY;

	bool mKeys[256];
};

#endif