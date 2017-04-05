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

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	void GetMouseLocation(int&, int&);

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

	IDirectInput8* mpDirectInput;
	IDirectInputDevice8* mpKeyboard;
	IDirectInputDevice8* mpMouse;

	unsigned char mKeyboardState[256];
	DIMOUSESTATE mMouseState;

	int mScreenWidth, mScreenHeight;
	int mMouseX, mMouseY;

};

#endif