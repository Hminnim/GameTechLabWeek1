#include "pch.h"
#include "UWindow.h"
#include "UInputManager.h"

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

//  Window Message Handler
LRESULT UWindow::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    switch (message)
    {
    // Mouse Button
    case WM_LBUTTONDOWN:
        SetCapture(hWnd);
        UInputManager::GetInstance().OnKeyDown(VK_LBUTTON);
        break;
    case WM_RBUTTONDOWN:
        SetCapture(hWnd);
        UInputManager::GetInstance().OnKeyDown(VK_RBUTTON);
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        UInputManager::GetInstance().OnKeyUp(VK_LBUTTON);
        break;
    case WM_RBUTTONUP:
        ReleaseCapture();
        UInputManager::GetInstance().OnKeyUp(VK_RBUTTON);
        break;

    case WM_MOUSEMOVE:
        int x, y;
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        UInputManager::GetInstance().OnMouseMove(x, y);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
