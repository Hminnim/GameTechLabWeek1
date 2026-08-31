#pragma once
#include "pch.h"

class UWindow
{
public:
	HWND hWnd = nullptr;

	bool InitializedWindow(HINSTANCE hInstance, WCHAR* WindowClass, WCHAR* WindowTitle, int Width, int Height);

	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
