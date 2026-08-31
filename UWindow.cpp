#include "pch.h"
#include "UWindow.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool UWindow::InitializedWindow(HINSTANCE hInstance, WCHAR* WindowClass, WCHAR* WindowTitle, int Width, int Height)
{
    WNDCLASS wndclass = { 0, UWindow::StaticWndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

    RegisterClassW(&wndclass);

    hWnd = CreateWindowExW(0, WindowClass, WindowTitle, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, Width, Height,
        nullptr, nullptr, hInstance, nullptr);

	return (hWnd != nullptr);
}

LRESULT UWindow::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
