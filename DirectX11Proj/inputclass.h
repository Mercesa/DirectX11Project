#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

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

	void RMouseDown()
	{
		if (rMouseDownC == false)
		{
			rMouseDown = true;
		}

		rMouseDownC = true;
	}

	void RMouseUp()
	{
		rMouseDownC = false;
		rMouseDown = false;
	}

	void KeyDown(unsigned int input)
	{
		// If a key is pressed then save that state in the key array.
		if (mKeysC[input] == false)
		{
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


	bool rMouseDown = false;
	bool rMouseDownC = false;

private:
	void ProcessInput();

	int mMouseX, mMouseY;
	int mMouseRelX, mMouseRelY;


	bool mKeys[256];
	bool mKeysC[256];
};

#endif