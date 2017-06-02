#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>
#include <iostream>

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

	// Don't want my if statements to be compiled away
#pragma optimize( "", off )

	void KeyDown(unsigned int input)
	{
		// If a key is pressed then save that state in the key array.
		if (mKeysC[input] == false)
		{
			std::cout << "Key pressed " << input << std::endl;
			mKeys[input] = true;
		}

		mKeysC[input] = true;
	}

	void KeyUp(unsigned int input)
	{
		// If a key is released then clear that state in the key array.
		mKeys[input] = false;
		mKeysC[input] = false;
	}
#pragma optimize( "", on ) 



	bool IsKeyDown(unsigned int key)
	{
		// Return what state the key is in (pressed/not pressed).
		return mKeys[key];
	}

	bool IsKeyHeld(unsigned int key)
	{
		return mKeysC[key];
	}

private:
	void ProcessInput();

	int mMouseX, mMouseY;
	int mMouseRelX, mMouseRelY;

	bool mKeys[256];
	bool mKeysC[256];
};

#endif